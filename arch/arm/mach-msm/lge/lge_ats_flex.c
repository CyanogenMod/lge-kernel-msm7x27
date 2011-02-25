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

 //LGE_CHAGE[irene.park@lge.com] 2010-06- 04 - to get flex value from ARM9 
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>
#include <mach/board_lge.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>

#include <linux/slab.h>

#include "lge_ats.h"
#include "lge_ats_flex.h"

int lg_get_flex_from_xml(char *strIndex, char* flexValue)
{
	int                       fd;
	int                       iFlexSize;
	signed long int      res;
	int                       iTotalCnt, iItemCnt,iValueCnt,j,iItemCntAll;
		
	char*                    s_bufFlexINI; //Flex INI ?????? ??ü string
	char                    s_bufItem[500];     //one line
	// char                    s_bufValue1[10];   //country
	char                    s_bufValue2[400]; //value
	char                    s_bufValue3[100]; //ID

	iFlexSize = 500;	
	fd = sys_open((const char __user *) "/system/etc/flex/flex.xml", O_RDONLY,0);

	if (fd == -1)	
		{
		printk("read data fail");
		return 0;
		}

	//iFlexSize = (unsigned)sys_lseek(fd, (off_t)0, 0);

	
	s_bufFlexINI = kmalloc(500, GFP_KERNEL);
	//memset(s_bufFlexINI,0x00,sizeof(char)*iFlexSize);	
	res = sys_read(fd, (char __user*)s_bufFlexINI, sizeof(char)*iFlexSize);
	
	sys_close(fd);

	printk("read data flex.xml fd: %d iFlexSize : %d res;%ld\n", fd, iFlexSize, res);

	iFlexSize=res;
	
	iItemCnt = 0;
	iItemCntAll = 0;
	
	for(iTotalCnt=0; iTotalCnt<iFlexSize;iTotalCnt++)  //Flex ini ?????? ?? character ??n?1?
	{
		//printk("%x ",s_bufFlexINI[iTotalCnt]);
		if ((s_bufFlexINI[iItemCntAll]) != '\n')  //???ڿ?; ???? ?о ???ۿ? ?ֱ?
		{
			s_bufItem[iItemCnt]=s_bufFlexINI[iItemCntAll];
			iItemCnt ++;
			
		} 
		else  //?о??? ???ڿ? ?м?
		{	
			//printk("\n",s_bufFlexINI[iTotalCnt]);
			s_bufItem[iItemCnt]='\n';
				
			j = 0;
			iValueCnt = 0;
			memset(s_bufValue3,0x00,sizeof(char)*100);
			while((s_bufItem[j] != '=') && (s_bufItem[j] != '\n') /*&& (s_bufItem[j] != ';')*/)  
			{
				//printk("\ns_bufValue3 ");
				if(s_bufItem[j] != ' ' && s_bufItem[j] != '\t') 
				{
					s_bufValue3[iValueCnt++] = s_bufItem[j];
					//printk("%x ",s_bufValue3[iValueCnt]);
				}
				j++;
			}

			if(!strncmp(s_bufValue3,strIndex,strlen(strIndex)))
			{ 
				//printk("find %s ",strIndex);
				iValueCnt = 0;
				j++;
				while(s_bufItem[j] != '\n' )  
				{
					//printk("\ns_bufItem ");
					if(s_bufItem[j] != '"') 
					{
						s_bufValue2[iValueCnt++] = s_bufItem[j];
						//printk("%x ",s_bufValue2[iValueCnt]);
					}
					j++;
				}
				memcpy(flexValue,s_bufValue2, iValueCnt);
				if(flexValue[iValueCnt-1] == '\r')
				{
					flexValue[iValueCnt-1] = 0x00;
					if(iValueCnt == 1)
					{
						printk("\niValueCnt == 1");
						return 0;
					}
				}
				else
				{
					flexValue[iValueCnt] = 0x00;
					if(iValueCnt == 0)
					{
						printk("\niValueCnt == 0");
						return 0;
					}
				}
				
				return 1;
			}
			iItemCnt = 0;
		}
		iItemCntAll++;
	}
	return 0;

}
//LGE_CHANGE_E [seypark@lge.com]

int lge_ats_handle_flex(struct msm_rpc_server *server,
							 struct rpc_request_hdr *req, unsigned len
							  )
{
	int result = HANDLE_OK;

	memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));

	switch (req->procedure)
	{
		case ONCRPC_LGE_GET_FLEX_MCC_PROC	:
		{		
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MCC_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_MCC_CODE", server->retvalue.ret_string))
				result = HANDLE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MCC_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 	
			break;
		}
		case ONCRPC_LGE_GET_FLEX_MNC_PROC	:
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_MNC_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_MNC_CODE", server->retvalue.ret_string))
				result = HANDLE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MNC_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 
			break;	
		}
		case ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC :
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_OPERATOR_CODE", server->retvalue.ret_string))
				result = HANDLE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 
			//LGE_CHANGE_S [bluerti@lge.com] 2009-08-17 <Activate fb_refresh during hidden reset > 
	#if 0
			if (!msm_fb_refesh_enabled && !fb_control_timer_init) { 
				printk("[Blue Debug] Set Timer\n");
				setup_timer(&lg_fb_control, lg_fb_control_timer, 0);	// LGE_CHANGE [bluerti@lge.com] 2009-10-8
		
				if( strncmp(server->retvalue.ret_string,"ORG",3) ==0 ) { //ORG
					mod_timer (&lg_fb_control,jiffies + (ORG_FB_TIMEOUT * HZ / 1000) ); //15sec
				} else {
					mod_timer (&lg_fb_control,jiffies + (OTHER_FB_TIMEOUT * HZ / 1000) );	//3sec
				}
		
				fb_control_timer_init = 1;
			}
	#endif
			//LGE_CHANGE_E [bluerti@lge.com]
		
			break;
		}
		case ONCRPC_LGE_GET_FLEX_COUNTRY_CODE_PROC	:
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_COUNTRY_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_COUNTRY_CODE", server->retvalue.ret_string))
				result = HANDLE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_COUNTRY_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 	
			break;
		}

		default :
			result = HANDLE_ERROR;
			break;
	}

        /////////////////////////////////////////////////////////////////
	
	if(result == HANDLE_OK)
					
		result = RPC_RETURN_RESULT_OK;
	else
		result= RPC_RETURN_RESULT_ERROR;
	
	printk(KERN_INFO "lge_ats_handle_flex result : %d \n",result);

	return result;
}
