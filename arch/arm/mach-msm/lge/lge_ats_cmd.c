/* arch/arm/mach-msm/lge/lge_ats_atcmd.c
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>
#include <mach/board_lge.h>
#include "lge_ats.h"
#include "lge_ats_cmd.h"

int lge_ats_handle_atcmd(struct msm_rpc_server *server,
							 struct rpc_request_hdr *req, unsigned len,
							 void (*update_atcmd_state)(char *cmd, int state) )
{
	struct rpc_ats_atcmd_args *args = (struct rpc_ats_atcmd_args *)(req + 1);
	int result = HANDLE_OK;
	int loop = 0;
	char ret_string[MAX_STRING_RET];
	uint32_t ret_value1 =0;
	uint32_t ret_value2 = 0;
	uint32_t at_cmd;
	uint32_t at_act;
	uint32_t at_param;

	at_cmd = be32_to_cpu(args->at_cmd);
	at_act = be32_to_cpu(args->at_act);
	at_param = be32_to_cpu(args->at_param);
	printk(KERN_INFO "%s: at_cmd = %d, at_act=%d, at_param=%d:\n",
		   __func__, args->at_cmd, args->at_act,args->at_param);

#ifdef USE_REPLY_RETSTRING
	memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));
#endif

	memset (ret_string, 0, sizeof(ret_string));

	switch (at_cmd)
	{
		case ATCMD_ACS:	//31
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;
			update_atcmd_state("acs", at_param); //state is up? down?
			break;
		case ATCMD_VLC:	//36
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;
			update_atcmd_state("vlc", at_param); //state is up? down?
			break;
		case ATCMD_SPM:	//40
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;
			update_atcmd_state("spm", at_param); //state is up? down?
			break;
		case ATCMD_MPT:	//43
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;
			update_atcmd_state("mpt", at_param); //state is up? down?
			break;
		case ATCMD_FMR: //42
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;
			update_atcmd_state("fmr", at_param); //state is up? down?
			break;

		case ATCMD_EMT:  // 46
			ret_value1 = external_memory_test();
			break;

		case ATCMD_FC:  // 59
			if(at_act != ATCMD_ACTION)             
				result = HANLDE_FAIL;
			ret_value1 = 0;
			break;
			
		case ATCMD_FO:  // 60                  
			if(at_act != ATCMD_ACTION)             
				result = HANLDE_FAIL;
			ret_value1 = 0;
			break;

		default :
			result = HANDLE_ERROR;
			break;
	}

	 /* give to RPC server result */
	for(loop = 0; loop < MAX_STRING_RET ; loop++)
	{
		server->retvalue.ret_string[loop]= (AT_STR_t)(ret_string[loop]);
	}                     
	server->retvalue.ret_string[MAX_STRING_RET-1] = 0;
	server->retvalue.ret_value1 = ret_value1;
	server->retvalue.ret_value2 = ret_value2;
        /////////////////////////////////////////////////////////////////
	
	if(result == HANDLE_OK)
					
		result = RPC_RETURN_RESULT_OK;
	else
		result= RPC_RETURN_RESULT_ERROR;

	return result;
}
