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
 */

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include "apr_audio.h"


#define TIMEOUT_MS 1000

struct adm_ctl {
	int ref_cnt;
	void *apr;
	int copp_id[AFE_MAX_PORTS];
	int copp_cnt[AFE_MAX_PORTS];
	int copp_state[AFE_MAX_PORTS];
	int copp_test[AFE_MAX_PORTS];
	wait_queue_head_t wait;
	struct mutex lock;
};

static struct adm_ctl this_adm;

static int32_t adm_callback(struct apr_client_data *data, void *priv)
{
	uint32_t *ptr;
	uint32_t opcode;
	ptr = data->payload;
	pr_debug("%s code = 0x%x %x %x size = %d\n", __func__,
			data->opcode, ptr[0], ptr[1], data->payload_size);

	if (data->payload_size) {
		if (data->opcode == APR_BASIC_RSP_RESULT) {
			opcode = ptr[0];
			this_adm.copp_test[data->token] = 1;
			wake_up(&this_adm.wait);
		}
		if (data->opcode == ADM_CMDRSP_COPP_OPEN) {
			struct adm_copp_open_respond *open = data->payload;
			this_adm.copp_id[data->token] = open->copp_id;
			this_adm.copp_state[data->token] = 1;
			wake_up(&this_adm.wait);
		}
	}

	return 0;
}

int adm_open(int port_id, int session_id , int path)
{
	struct adm_copp_open_command open;
	struct adm_routings_command route;
	int rc;

	pr_info("%s port %d session 0x%x\n", __func__, port_id, session_id);

	if (port_id >= AFE_MAX_PORTS)
		return -ENODEV;

	mutex_lock(&this_adm.lock);
	if (this_adm.ref_cnt == 0) {
		this_adm.apr = apr_register("ADSP", "ADM", adm_callback,
						0xFFFFFFFF, &this_adm);
		if (this_adm.apr == NULL) {
			pr_err("Unable to register ADM\n");
			goto fail;
		}
	}

	/* Create a COPP if port id are not enabled */
	if (this_adm.copp_cnt[port_id] == 0) {

		open.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		open.hdr.pkt_size = sizeof(open);
		open.hdr.src_svc = APR_SVC_ADM;
		open.hdr.src_domain = APR_DOMAIN_APPS;
		open.hdr.src_port = port_id;
		open.hdr.dest_svc = APR_SVC_ADM;
		open.hdr.dest_domain = APR_DOMAIN_ADSP;
		open.hdr.dest_port = port_id;
		open.hdr.token = port_id;
		open.hdr.opcode = ADM_CMD_COPP_OPEN;
		open.flags = 1; /* 1 = real time, 0 = non-real time */
		open.endpoint_id = port_id;
		open.topology_id = DEFAULT_TOPOLOGY;

		this_adm.copp_state[port_id] = 0;

		rc = apr_send_pkt(this_adm.apr, (uint32_t *)&open);
		if (rc < 0) {
			pr_err("ADM enable for port %d failed\n", port_id);
			goto fail_cmd;
		}
		/* Wait for the callback with copp id */
		rc = wait_event_timeout(this_adm.wait,
			this_adm.copp_state[port_id],
			msecs_to_jiffies(TIMEOUT_MS));
		if (rc < 0) {
			pr_info("ADM open failed for port %d\n", port_id);
			goto fail_cmd;
		}
	}
	this_adm.copp_cnt[port_id]++;


	if (this_adm.copp_state[port_id]) {

		route.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		route.hdr.pkt_size = sizeof(route);
		route.hdr.src_svc = 0;
		route.hdr.src_domain = APR_DOMAIN_APPS;
		route.hdr.src_port = this_adm.copp_id[port_id];
		route.hdr.dest_svc = APR_SVC_ADM;
		route.hdr.dest_domain = APR_DOMAIN_ADSP;
		route.hdr.dest_port = this_adm.copp_id[port_id];
		route.hdr.token = port_id;
		route.hdr.opcode = ADM_CMD_MATRIX_MAP_ROUTINGS;
		route.path = path;
		route.num_sessions = 1;
		route.sessions[0].id = session_id;
		route.sessions[0].num_copps = 1;
		route.sessions[0].copp_id[0] = this_adm.copp_id[port_id];

		this_adm.copp_test[port_id] = 0;
		this_adm.copp_state[port_id] = 0;

		rc = apr_send_pkt(this_adm.apr, (uint32_t *)&route);

		if (rc < 0) {
			pr_err("ADM routing for port %d failed\n", port_id);
			goto fail_cmd;
		}

		rc = wait_event_timeout(this_adm.wait,
				this_adm.copp_test[port_id],
				msecs_to_jiffies(TIMEOUT_MS));
		if (rc < 0) {
			pr_info("ADM cmd Route failed for port %d\n", port_id);
			goto fail_cmd;
		}
	}

	this_adm.ref_cnt++;
	mutex_unlock(&this_adm.lock);
	return 0;

fail_cmd:
	if (this_adm.ref_cnt == 0)
		apr_deregister(this_adm.apr);
fail:
	mutex_unlock(&this_adm.lock);
	return -EINVAL;
}


int adm_close(int port_id)
{
	struct apr_hdr close;

	int ret;

	pr_info("%s\n", __func__);

	mutex_lock(&this_adm.lock);
	if (this_adm.ref_cnt <= 0) {
		pr_err("ADM is already closed\n");
		mutex_unlock(&this_adm.lock);
		return -EINVAL;
	}


	this_adm.copp_cnt[port_id]--;
	if (!this_adm.copp_cnt[port_id]) {

		close.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		close.pkt_size = sizeof(close);
		close.src_svc = APR_SVC_ADM;
		close.src_domain = APR_DOMAIN_APPS;
		close.src_port = port_id;
		close.dest_svc = APR_SVC_ADM;
		close.dest_domain = APR_DOMAIN_ADSP;
		close.dest_port = this_adm.copp_id[port_id];
		close.token = port_id;
		close.opcode = ADM_CMD_COPP_CLOSE;

		this_adm.copp_id[port_id] = 0;
		this_adm.copp_state[port_id] = 0;
		this_adm.copp_test[port_id] = 0;

		ret = apr_send_pkt(this_adm.apr, (uint32_t *)&close);
		if (ret < 0) {
			pr_err("ADM close failed\n");
			mutex_unlock(&this_adm.lock);
			return -EINVAL;
		}

		ret = wait_event_timeout(this_adm.wait,
				this_adm.copp_test[port_id],
				msecs_to_jiffies(TIMEOUT_MS));
		if (ret < 0) {
			pr_info("ADM cmd Route failed for port %d\n", port_id);
			mutex_unlock(&this_adm.lock);
			return -EINVAL;
		}
	}

	this_adm.ref_cnt--;
	if (this_adm.ref_cnt == 0)
		apr_deregister(this_adm.apr);
	mutex_unlock(&this_adm.lock);

	return 0;
}

static int __init adm_init(void)
{
	pr_info("%s\n", __func__);
	init_waitqueue_head(&this_adm.wait);
	mutex_init(&this_adm.lock);
	return 0;
}

device_initcall(adm_init);
