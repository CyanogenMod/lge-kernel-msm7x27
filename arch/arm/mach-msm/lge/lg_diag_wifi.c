/*
  Program : UDM 

  Author : khlee

  Date : 2010.01.26
*/
/* ==========================================================================
===========================================================================*/
#include <linux/module.h>
#include <mach/lg_diag_wifi.h>
#include <mach/lg_diagcmd.h>
#include <mach/lg_diag_testmode.h>

/* ==========================================================================
===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
/*==========================================================================*/

PACK (void *)LGF_WIFI(
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
  DIAG_LGE_WIFI_MAC_ADDRESS_req_tag	*req_ptr = (DIAG_LGE_WIFI_MAC_ADDRESS_req_tag *) req_pkt_ptr;
  DIAG_LGE_WIFI_MAC_ADDRESS_rsp_tag	*rsp_ptr = NULL;
//  unsigned int rsp_len;

//  rsp_len = sizeof(DIAG_LGE_WIFI_MAC_ADDRESS_rsp_tag);		
//  rsp_ptr = (DIAG_LGE_WIFI_MAC_ADDRESS_rsp_tag *)diagpkt_alloc(DIAG_WIFI_MAC_ADDR, rsp_len);
//  rsp_ptr->nrb_udm_mode.sub_cmd_code = req_ptr->nrb_udm_mode.sub_cmd_code;
//  rsp_ptr->result = TEST_OK_S;

  printk(KERN_ERR "[WIFI] SubCmd=<%d>\n",req_ptr->sub_cmd);

  switch( req_ptr->sub_cmd )
  {

    default:
//      rsp_ptr->nrb_udm_mode.ret_stat_code = TEST_FAIL_S;
      break;
  }

  return (rsp_ptr);	
}

EXPORT_SYMBOL(LGF_WIFI); 
