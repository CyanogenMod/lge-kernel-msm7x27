/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/radix-tree.h>
#include <mach/clk.h>
#include <mach/board.h>
#include "msm_bus_core.h"
#include "rpm.h"

#define BWMASK 0x7FFF
#define TIERMASK 0x8000
#define GET_TIER(n) (((n) & TIERMASK) >> 15)

struct msm_bus_fabric {
	struct msm_bus_fabric_device fabdev;
	int ahb;
	int nmasters;
	int nslaves;
	int ntieredslaves;
	struct commit_data *cdata;
	int dirty;
	struct radix_tree_root fab_tree;
	int num_nodes;
	struct list_head gateways;
	struct msm_bus_inode_info info;
	const struct msm_bus_fab_algorithm *algo;
	struct msm_bus_fabric_registration *pdata;
};
#define to_msm_bus_fabric(d) container_of(d, \
	struct msm_bus_fabric, d)
/**
 * msm_bus_fabric_add_node() - Add a node to the fabric structure
 * @fabric: Fabric device to which the node should be added
 * @info: The node to be added
 */
static int msm_bus_fabric_add_node(struct msm_bus_fabric *fabric,
	struct msm_bus_inode_info *info)
{
	int status = -ENOMEM;
	MSM_FAB_DBG("msm_bus_fabric_add_node: ID %d Gw: %d\n",
		info->node_info->id, info->node_info->gateway);
	status = radix_tree_preload(GFP_KERNEL);
	if (status)
		goto out;
	status = radix_tree_insert(&fabric->fab_tree, info->node_info->id,
			info);
	radix_tree_preload_end();
out:
	return status;
}

/**
 * msm_bus_add_fab() - Add a fabric (gateway) to the current fabric
 * @fabric: Fabric device to which the gateway info should be added
 * @info: Gateway node to be added to the fabric
 */
static int msm_bus_fabric_add_fab(struct msm_bus_fabric *fabric,
	struct msm_bus_inode_info *info)
{
	int status = 0;
	struct msm_bus_fabnodeinfo *fabnodeinfo;
	MSM_FAB_DBG("msm_bus_fabric_add_fab: ID %d Gw: %d\n",
		info->node_info->id, info->node_info->gateway);
	fabnodeinfo = kzalloc(sizeof(struct msm_bus_fabnodeinfo), GFP_KERNEL);
	if (!fabnodeinfo) {
		MSM_FAB_ERR("msm_bus_fabric_add_fab: "
			"No Node Info\n");
		MSM_FAB_ERR("axi: Cannot register fabric!\n");
		status = -ENOMEM;
	}

	fabnodeinfo->info = info;
	fabnodeinfo->info->num_pnodes = -1;
	list_add_tail(&fabnodeinfo->list, &fabric->gateways);
	return status;
}

/**
 * register_fabric_info() - Create the internal fabric structure and
 * build the topology tree from platform specific data
 *
 * @fabric: Fabric to which the gateways, nodes should be added
 *
 * This function is called from probe. Iterates over the platform data,
 * and builds the topology
 */
static int register_fabric_info(struct msm_bus_fabric *fabric)
{
	int i, ret = 0, err = 0;

	MSM_FAB_DBG("id:%d pdata-id: %d len: %d\n", fabric->fabdev.id,
		fabric->pdata->id, fabric->pdata->len);

	for (i = 0; i < fabric->pdata->len; i++) {
		struct msm_bus_inode_info *info;
		info = kzalloc(sizeof(struct msm_bus_inode_info), GFP_KERNEL);
		info->node_info = fabric->pdata->info++;
		info->commit_index = -1;
		info->num_pnodes = -1;
		if (info->node_info->slaveclk) {
			info->nodeclk = clk_get(NULL, info->node_info->
					slaveclk);
			if (IS_ERR(info->nodeclk)) {
				MSM_BUS_ERR("Could not get clock for %s\n",
					info->node_info->slaveclk);
				err = -EINVAL;
			}
		}
		if (info->node_info->memclk) {
			info->memclk = clk_get(NULL,
					info->node_info->memclk);
			if (IS_ERR(info->memclk)) {
				MSM_BUS_ERR("Could not get clock for %s\n",
					info->node_info->slaveclk);
				err = -EINVAL;
			}
		}

		ret = info->node_info->gateway ?
			msm_bus_fabric_add_fab(fabric, info) :
			msm_bus_fabric_add_node(fabric, info);
		if (ret) {
			MSM_BUS_ERR("Unable to add node info, ret: %d\n", ret);
			kfree(info);
			goto error;
		}

		if (info->node_info->masterp && !(info->node_info->ahb))
			fabric->nmasters++;
		if ((!info->node_info->ahb) && (info->node_info->gateway ||
			IS_SLAVE(info->node_info->id))) {
			if (info->node_info->tier)
				fabric->ntieredslaves++;
			fabric->nslaves++;
		}
	}
	MSM_FAB_DBG("Fabric: %d nmasters: %d nslaves: %d\n"
		" ntieredslaves: %d\n", fabric->fabdev.id, fabric->nmasters,
		fabric->nslaves, fabric->ntieredslaves);
	MSM_FAB_DBG("msm_bus_register_fabric_info i: %d\n", i);
	fabric->num_nodes = fabric->pdata->len;
error:
	fabric->num_nodes = i;
	return ret | err;
}

/**
 * msm_bus_fabric_rpm_commit() - Commit the arbitration data to RPM
 * @fabric: Fabric for which the data should be committed
 * */
static int msm_bus_fabric_rpm_commit(struct msm_bus_fabric_device *fabdev)
{
	int i, j, offset = 0, status = 0, count;
	struct msm_rpm_iv_pair *rpm_data;
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);
	/*
	 * count is the number of 2-byte words required to commit the
	 * data to rpm. This is calculated by the following formula.
	 * Commit data is split into two arrays:
	 * 1. bwsum[nmasters][ntieredslaves]
	 * 2. arb[nslaves]
	 */
	if (!fabric->dirty) {
		MSM_FAB_DBG("Not committing as fabric not dirty\n");
		return status;
	}
	count = ((fabric->nmasters * fabric->ntieredslaves)
		+ (fabric->nslaves))/2;

	rpm_data = kmalloc((sizeof(struct msm_rpm_iv_pair) * count),
			GFP_KERNEL);

	offset = fabric->pdata->offset;
	for (i = 0; i < count; i++) {
		rpm_data[i].id = offset + i;
		rpm_data[i].value = *((uint32_t *)fabric->cdata + i);
	}

	MSM_FAB_DBG("Commit Data: Fab: %d BWSum:\n", fabric->fabdev.id);
	for (i = 0; i < fabric->nslaves; i++)
		MSM_FAB_DBG("fab_slaves:0x%x\n",
			fabric->cdata->bwsum[i]);
	MSM_FAB_DBG("Commit Data: Fab: %d Arb:\n", fabric->fabdev.id);
	for (i = 0; i < fabric->ntieredslaves; i++) {
		MSM_FAB_DBG("tiered-slave: %d\n", i);
		for (j = 0; j < fabric->nmasters; j++)
			MSM_FAB_DBG(" 0x%x\n",
			fabric->cdata->arb[i][j]);
	}

	MSM_FAB_DBG("calling msm_rpm_set:  %d\n", status);
	status = msm_rpm_set(MSM_RPM_CTX_SET_0, rpm_data, count);
	MSM_FAB_DBG("msm_rpm_set returned: %d\n", status);
	fabric->dirty = false;
	kfree(rpm_data);
	return status;
}

/**
 * msm_bus_fabric_update_clks() - Set the clocks for fabrics and slaves
 * @fabric: Fabric for which the clocks need to be updated
 * @slave: The node for which the clocks need to be updated
 * @index: The index for which the current clocks are set
 * @curr_clk:Current clock value
 * @req_clk: Requested clock value
 * @bwsum: Bandwidth Sum
 * @clk_sel: Flag determining whether fabric clock or the slave clock has to
 * be set. If clk_sel is set, fabric clock is set, else slave clock is set.
 */
static int msm_bus_fabric_update_clks(struct msm_bus_fabric_device *fabdev,
		struct msm_bus_inode_info *slave, int index,
		unsigned long curr_clk, unsigned long req_clk,
		unsigned long bwsum, int clk_sel)
{
	int i, status = 0;
	unsigned long max_pclk = 0;
	unsigned long *pclk = NULL;
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);
	/* Maximum for this gateway */
	for (i = 0; i <= slave->num_pnodes; i++) {
		if (i == index && (req_clk < curr_clk))
			continue;
		max_pclk = max(max_pclk, slave->pnode[i].clk);
	}
	slave->link_info.clk = max(max_pclk, max(
		(unsigned long)BW_TO_CLK_FREQ_KHZ(slave->node_info->buswidth,
		bwsum), req_clk));
	/* Is this gateway or slave? */
	if (clk_sel && (!fabric->ahb)) {
		struct msm_bus_fabnodeinfo *fabgw = NULL;
		struct msm_bus_inode_info *info = NULL;
		/* Maximum of all gateways set at fabric */
		list_for_each_entry(fabgw, &fabric->gateways, list) {
			info = fabgw->info;
			max_pclk = max(max_pclk, info->link_info.clk);
		}
		MSM_FAB_DBG("max_pclk from gateways: %lu\n", max_pclk);

		/* Maximum of all slave clocks. */
		for (i = 0; i < fabric->nslaves; i++) {
			info = radix_tree_lookup(&fabric->fab_tree,
				(fabric->fabdev.id + SLAVE_ID_KEY + i));
			if (!info)
				continue;
			max_pclk = max(max_pclk, info->link_info.clk);
		}
		MSM_FAB_DBG("max_pclk from slaves & gws: %lu\n", max_pclk);
		pclk = &fabric->info.link_info.clk;
	} else
		pclk = &slave->link_info.clk;

	if (!slave->node_info->buswidth)
		MSM_FAB_DBG("Invalid width!, using default width 8\n");
	else
		slave->node_info->buswidth = 8;

	*pclk = max(max_pclk, max((unsigned long)BW_TO_CLK_FREQ_KHZ(slave->
		node_info->buswidth, bwsum), req_clk));

	if (clk_sel) {
		MSM_FAB_DBG("AXI_clks: id: %d set-clk: %lu bwsum:%lu\n",
			fabric->fabdev.id, *pclk, bwsum);
		if (fabric->info.nodeclk)
			status = clk_set_min_rate(fabric->info.nodeclk, *pclk);
	} else {
		MSM_FAB_DBG("AXI_clks: id: %d set-clk: %lu  bwsum:%lu\n" ,
			slave->node_info->id, *pclk, bwsum);
		if (slave->nodeclk) {
			status = clk_set_min_rate(slave->nodeclk, *pclk);
			MSM_BUS_DBG("Trying to set clk, node id: %d val: %lu "
				"status %d\n", slave->node_info->id, *pclk,
				status);
		}
		if (!status && slave->memclk)
			status = clk_set_min_rate(slave->memclk,
			slave->link_info.clk);
	}
	return status;
}

void msm_bus_fabric_update_bw(struct msm_bus_fabric_device *fabdev,
	struct msm_bus_inode_info *hop, struct msm_bus_inode_info *info,
	int add_bw, int master_tier)
{
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);
	/* If it's an ahb fabric, don't calculate arb values */
	if (fabric->ahb) {
		MSM_FAB_DBG("AHB fabric, skipping bw calculation\n");
		return;
	}
	/* If no tier, set it to default value */
	if (hop->link_info.tier == 0)
		hop->link_info.tier = MSM_BUS_BW_TIER2;
	/* If there is tier, calculate arb for commit */
	if (hop->node_info->tier) {
		uint16_t tier;
		uint16_t tieredbw = (fabric->cdata->arb[hop->node_info->tier
			- 1][info->node_info->masterp - 1] & BWMASK);
		if (GET_TIER(fabric->cdata->arb[hop->node_info->tier - 1]
			[info->node_info->masterp - 1]))
			tier = MSM_BUS_BW_TIER1;
		else
			tier = master_tier;
		tieredbw += add_bw;
		/* If bw is 0, update tier to default */
		if (!tieredbw)
			tier = MSM_BUS_BW_TIER2;
		/* Update Arb for fab,get HW Mport from enum */
		fabric->cdata->arb[hop->node_info->tier - 1]
			[info->node_info->masterp - 1] = (uint16_t)
			CREATE_BW_TIER_PAIR(tier, tieredbw);
		MSM_BUS_DBG("tier:%d mport: %d add_bw:%d bwsum: %d\n",
			hop->node_info->tier - 1, info->node_info->masterp - 1,
			add_bw, hop->link_info.bw);
	}
	/* Update bwsum for slaves on fabric */
	fabric->cdata->bwsum[hop->node_info->slavep]
		= (uint16_t)hop->link_info.bw;
	MSM_BUS_DBG("slavep:%d, link_bw: %d\n",
		hop->node_info->slavep, hop->link_info.bw);
	fabric->dirty = true;
}

/**
 * msm_bus_fabric_port_halt() - Used to halt a master port
 * @fabric: Fabric on which the current master node is present
 * @portid: Port id of the master
 */
int msm_bus_fabric_port_halt(struct msm_bus_fabric_device *fabdev, int portid)
{
	struct msm_bus_halt_vector hvector = {0, 0};
	struct msm_rpm_iv_pair rpm_data[2];
	uint8_t mport = (uint8_t)GET_MPORT(portid);
	uint32_t id = 0;
	int status = 0;
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);

	MSM_BUS_MASTER_HALT(hvector.haltmask, hvector.haltval, mport);
	id = fabric->pdata->haltid;

	rpm_data[0].id = id;
	rpm_data[0].value = hvector.haltval;
	rpm_data[1].id = id + 1;
	rpm_data[1].value = hvector.haltmask;

	MSM_FAB_DBG("ctx: %d, id: %d, value: %d\n",
			MSM_RPM_CTX_SET_0,
			rpm_data[0].id, rpm_data[0].value);
	MSM_FAB_DBG("ctx: %d, id: %d, value: %d\n",
			MSM_RPM_CTX_SET_0,
			rpm_data[1].id, rpm_data[1].value);

	status = msm_rpm_set(MSM_RPM_CTX_SET_0, rpm_data, 2);
	MSM_FAB_DBG("msm_rpm_set returned: %d\n", status);
	return status;
}

/**
 * msm_bus_fabric_port_unhalt() - Used to unhalt a master port
 * @fabric: Fabric on which the current master node is present
 * @portid: Port id of the master
 */
int msm_bus_fabric_port_unhalt(struct msm_bus_fabric_device *fabdev, int portid)
{
	struct msm_bus_halt_vector hvector = {0, 0};
	struct msm_rpm_iv_pair rpm_data[2];
	uint8_t mport = (uint8_t)GET_MPORT(portid);
	uint32_t id = 0;
	int status = 0;
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);

	MSM_BUS_MASTER_UNHALT(hvector.haltmask, hvector.haltval, mport);
	id = fabric->pdata->haltid;

	rpm_data[0].id = id;
	rpm_data[0].value = hvector.haltval;
	rpm_data[1].id = id + 1;
	rpm_data[1].value = hvector.haltmask;

	MSM_FAB_DBG("unalt: ctx: %d, id: %d, value: %d\n",
			MSM_RPM_CTX_SET_SLEEP,
			rpm_data[0].id, rpm_data[0].value);
	MSM_FAB_DBG("unhalt: ctx: %d, id: %d, value: %d\n",
			MSM_RPM_CTX_SET_SLEEP,
			rpm_data[1].id, rpm_data[1].value);

	status = msm_rpm_set(MSM_RPM_CTX_SET_0, rpm_data, 2);
	MSM_FAB_DBG("msm_rpm_set returned: %d\n", status);
	return status;
}

/**
 * msm_bus_fabric_find_gw_node() - This function finds the gateway node
 * attached on a given fabric
 * @id:       ID of the gateway node
 * @fabric:   Fabric to find the gateway node on
 * Function returns: Pointer to the gateway node
 */
static struct msm_bus_inode_info *msm_bus_fabric_find_gw_node(struct
	msm_bus_fabric_device * fabdev, int id)
{
	struct msm_bus_inode_info *info = NULL;
	struct msm_bus_fabnodeinfo *fab;
	struct msm_bus_fabric *fabric;
	if (!fabdev) {
		MSM_BUS_ERR("No fabric device found!\n");
		return NULL;
	}

	fabric = to_msm_bus_fabric(fabdev);
	if (!fabric || IS_ERR(fabric)) {
		MSM_BUS_ERR("No fabric type found!\n");
		return NULL;
	}
	list_for_each_entry(fab, &fabric->gateways, list) {
		if (fab->info->node_info->id == id) {
			info = fab->info;
			break;
		}
	}

	return info;
}

static struct msm_bus_inode_info *msm_bus_fabric_find_node(struct
	msm_bus_fabric_device * fabdev, int id)
{
	struct msm_bus_inode_info *info = NULL;
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);
	info = radix_tree_lookup(&fabric->fab_tree, id);
	if (!info)
		MSM_FAB_DBG("Null info found for id %d\n", id);
	return info;
}

static struct list_head *msm_bus_fabric_get_gw_list(struct msm_bus_fabric_device
	*fabdev)
{
	struct msm_bus_fabric *fabric = to_msm_bus_fabric(fabdev);
	if (!fabric || IS_ERR(fabric)) {
		MSM_BUS_ERR("No fabric found from fabdev\n");
		return NULL;
	}
	return &fabric->gateways;

}
static struct msm_bus_fab_algorithm msm_bus_algo = {
	.update_clks = msm_bus_fabric_update_clks,
	.update_bw = msm_bus_fabric_update_bw,
	.port_halt = msm_bus_fabric_port_halt,
	.port_unhalt = msm_bus_fabric_port_unhalt,
	.commit = msm_bus_fabric_rpm_commit,
	.find_node = msm_bus_fabric_find_node,
	.find_gw_node = msm_bus_fabric_find_gw_node,
	.get_gw_list = msm_bus_fabric_get_gw_list,
};

/**
 * allocate_commit_data() - Allocate the data for commit array in the
 * format specified by RPM
 * @fabric: Fabric device for which commit data is allocated
 */
static int allocate_commit_data(struct msm_bus_fabric *fabric)
{
	int i;
	struct commit_data *cdata;

	cdata = kzalloc(sizeof(struct commit_data), GFP_KERNEL);
	if (!cdata) {
		MSM_FAB_DBG("Couldn't alloc mem for cdata\n");
		return -ENOMEM;
	}
	cdata->bwsum = kzalloc((sizeof(uint16_t) * fabric->nslaves),
			GFP_KERNEL);
	if (!cdata->bwsum) {
		MSM_FAB_DBG("Couldn't alloc mem for slaves\n");
		kfree(cdata);
		return -ENOMEM;
	}
	cdata->arb = kzalloc((sizeof(uint16_t *)) *
			fabric->ntieredslaves, GFP_KERNEL);
	for (i = 0; i < fabric->ntieredslaves; i++) {
		cdata->arb[i] = kzalloc((sizeof(uint16_t)) * fabric->nmasters,
				GFP_KERNEL);
		if (!cdata->arb[i]) {
			MSM_FAB_DBG("Couldn't alloc memory for"
					" slaves\n");
		kfree(cdata->bwsum);
		kfree(cdata);
		return -ENOMEM;
		}
	}
	fabric->cdata = cdata;
	return 0;
}

static int msm_bus_fabric_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct msm_bus_fabric *fabric;
	struct msm_bus_fabric_registration *pdata;

	fabric = kzalloc(sizeof(struct msm_bus_fabric), GFP_KERNEL);
	if (!fabric) {
		MSM_BUS_ERR("Fabric alloc failed\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&fabric->gateways);
	INIT_RADIX_TREE(&fabric->fab_tree, GFP_KERNEL);
	fabric->num_nodes = 0;
	fabric->fabdev.id = pdev->id;
	fabric->fabdev.visited = false;

	fabric->info.node_info = kzalloc(sizeof(struct msm_bus_node_info),
				GFP_KERNEL);
	if (!fabric->info.node_info) {
		MSM_BUS_ERR("Fabric node info alloc failed\n");
		kfree(fabric);
		return -ENOMEM;
	}
	fabric->info.node_info->id = fabric->fabdev.id;
	fabric->info.num_pnodes = -1;
	fabric->info.link_info.clk = 0;
	fabric->info.link_info.bw = 0;

	fabric->fabdev.id = pdev->id;
	pdata = (struct msm_bus_fabric_registration *)pdev->dev.platform_data;
	fabric->fabdev.name = pdata->name;
	fabric->fabdev.algo = &msm_bus_algo;
	fabric->ahb = pdata->ahb;
	fabric->pdata = pdata;

	if (pdata->fabclk) {
		fabric->info.nodeclk = clk_get(NULL, pdata->fabclk);
		if (IS_ERR(fabric->info.nodeclk)) {
			MSM_BUS_ERR("Could not get clock for %s\n",
				pdata->fabclk);
			ret = -EINVAL;
			goto err;
		}
	}

	/* Find num. of slaves, masters, populate gateways, radix tree */
	ret = register_fabric_info(fabric);
	if (ret) {
		MSM_BUS_ERR("Could not register fabric %d info, ret: %d\n",
			fabric->fabdev.id, ret);
		goto err;
	}
	if (!fabric->ahb) {
		/* Allocate memory for commit data */
		ret = allocate_commit_data(fabric);
		if (ret) {
			MSM_BUS_ERR("Failed to alloc commit data for fab: %d,"
				"ret = %d\n", fabric->fabdev.id, ret);
			goto err;
		}
	}
	/*
	 * clk and bw for fabric->info will contain the max bw and clk
	 * it will allow. This info will come from the boards file.
	 */
	ret = msm_bus_fabric_device_register(&fabric->fabdev);
	if (ret) {
		MSM_BUS_ERR("Error registering fabric %d ret %d\n",
			fabric->fabdev.id, ret);
		goto err;
	}
	return ret;
err:
	kfree(fabric->info.node_info);
	kfree(fabric);
	return ret;
}

static int msm_bus_fabric_remove(struct platform_device *pdev)
{
	struct msm_bus_fabric_device *fabdev = NULL;
	struct msm_bus_fabric *fabric;
	int i;
	int ret = 0;

	fabdev = platform_get_drvdata(pdev);
	msm_bus_fabric_device_unregister(fabdev);
	fabric = to_msm_bus_fabric(fabdev);
	for (i = 0; i < fabric->nmasters; i++)
		radix_tree_delete(&fabric->fab_tree, fabric->fabdev.id + i);
	for (i = (fabric->fabdev.id + SLAVE_ID_KEY); i < fabric->nslaves; i++)
		radix_tree_delete(&fabric->fab_tree, i);
	if (!fabric->ahb) {
		kfree(fabric->cdata->bwsum);
		for (i = 0; i < fabric->nmasters; i++)
			kfree(fabric->cdata->arb[i]);
		kfree(fabric->cdata->arb);
		kfree(fabric->cdata);
	}
	kfree(fabric->info.node_info);
	kfree(fabric);
	return ret;
}

static struct platform_driver msm_bus_fabric_driver = {
	.probe = msm_bus_fabric_probe,
	.remove = msm_bus_fabric_remove,
	.driver = {
		.name = "msm_bus_fabric",
		.owner = THIS_MODULE,
	},
};

static int __init msm_bus_fabric_init_driver(void)
{
	MSM_BUS_ERR("msm_bus_fabric_init_driver\n");
	return platform_driver_register(&msm_bus_fabric_driver);
}
arch_initcall(msm_bus_fabric_init_driver);
