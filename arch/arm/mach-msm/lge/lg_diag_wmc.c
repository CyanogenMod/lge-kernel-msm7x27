#include <linux/module.h>
#include <linux/delay.h>
#include <mach/lg_diagcmd.h>
#include <mach/lg_diag_wmc.h>

//#include "lg_fw_diag_communication.h" // diagcmd_dev

#include <linux/unistd.h> /*for open/close*/
#include <linux/fcntl.h> /*for O_RDWR*/
#include <linux/syscalls.h> //for sys operations
#include <linux/fs.h> // for file struct
/* ==========================================================================

                      EXTERNAL FUNCTION AND VARIABLE DEFINITIONS

===========================================================================*/
//extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
//extern unsigned int LGF_KeycodeTrans(word input);

/*==========================================================================

            LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE

  This section contains local definitions for constants, macros, types,
  variables and other items needed by this module.

===========================================================================*/
#ifdef CONFIG_LGE_DIAG_WMC

void* lg_diag_wmc_req_pkt_ptr;
uint16 lg_diag_wmc_req_pkt_length;
uint16 lg_diag_wmc_rsp_pkt_length;

PACK (void *)LGF_WMC (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{

  int ret;
  char cmdstr[100];
  int fd;
  char *envp[] = {
    "HOME=/",
    "TERM=linux",
    NULL,
  };

  char *argv[] = {
    "sh",
    "-c",
    cmdstr,
    NULL,
  };	

 // static struct diagcmd_dev *diagpdev;
  
  //printk(KERN_INFO "%s, pkt_len : %d\n",__func__, pkt_len);
  //printk(KERN_INFO "%s, cmd_code : 0x%x, sub_cmd : 0x%x\n",__func__, *(unsigned char *)(req_pkt_ptr + 0), *(unsigned char *)(req_pkt_ptr + 1));
  

  lg_diag_wmc_req_pkt_ptr = req_pkt_ptr;
  lg_diag_wmc_req_pkt_length = pkt_len;

/* Below codes are for diag observer, no need to add
  diagpdev = diagcmd_get_dev(); // get lg_fw_diagcmd dev

  if ((diagpdev != NULL)&& (pkt_len < 1024)){
    //printk(KERN_INFO "%s, name : %s, index : %d, state : %d\n",\
      __func__, (char *)diagpdev->name, diagpdev->index, diagpdev->state);

    update_diagcmd_state(diagpdev, "LG_DIAG_WMC", 0);
  }
  else 
  {
    //printk(KERN_ERR "%s, pkt_len : %d\n",__func__, pkt_len);
    return NULL;
  }
*/
  if ( (fd = sys_open((const char __user *) "/system/bin/lg_diag_wmc", O_RDONLY ,0) ) < 0 )
  {
    //printk("\n can not open /system/bin/lg_diag - execute /system/bin/lg_diag_wmc\n");
    sprintf(cmdstr, "/system/bin/lg_diag_wmc\n");
  }
  else
  {
    //printk("\n execute /system/bin/lg_diag_wmc\n");
    sprintf(cmdstr, "/system/bin/lg_diag_wmc\n");
    sys_close(fd);
  }

  //printk(KERN_INFO "execute - %s", cmdstr);

  if ((ret = call_usermodehelper("/system/bin/sh", argv, envp, UMH_WAIT_PROC)) != 0) {
  	//printk(KERN_ERR "%s, failed to run %s: %i\n", __func__, cmdstr, ret);
  }
  else
    //printk(KERN_INFO "%s, %s execute ok\n", __func__, cmdstr);
  
  return NULL;

}

EXPORT_SYMBOL(LGF_WMC);

#endif
