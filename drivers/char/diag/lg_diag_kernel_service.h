#ifndef LG_DIAG_KERNEL_SERVICE_H
#define LG_DIAG_KERNEL_SERVICE_H

#include <mach/lg_comdef.h>

//#define LG_DIAG_DEBUG

#define DIAGPKT_HDR_PATTERN (0xDEADD00DU)
#define DIAGPKT_OVERRUN_PATTERN (0xDEADU)
#define DIAGPKT_USER_TBL_SIZE 10
#define READ_BUF_SIZE 8004

#define DIAG_DATA_TYPE_EVENT         0
#define DIAG_DATA_TYPE_F3            1
#define DIAG_DATA_TYPE_LOG           2
#define DIAG_DATA_TYPE_RESPONSE      3
#define DIAG_DATA_TYPE_DELAYED_RESPONSE   4

#define DIAGPKT_NO_SUBSYS_ID 0xFF

#define TRUE 1
#define FALSE 0


/*********************** BEGIN PACK() Definition ***************************/
#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif
/********************** END PACK() Definition *****************************/

typedef struct
{
  word cmd_code_lo;
  word cmd_code_hi;
  PACK(void *)(*func_ptr) (PACK(void *)req_pkt_ptr, uint16 pkt_len);
} diagpkt_user_table_entry_type;

typedef struct
{
	 uint16 cmd_code;
	 uint16 subsys_id;
	 uint16 cmd_code_lo;
	 uint16 cmd_code_hi;
	 uint16 proc_id;
	 uint32 event_id;
	 uint32 log_code;
	 uint32 client_id;
} bindpkt_params;

#define MAX_SYNC_OBJ_NAME_SIZE 32
typedef struct
{
	 char sync_obj_name[MAX_SYNC_OBJ_NAME_SIZE]; /* Name of the synchronization object associated with this process */
	 uint32 count; /* Number of entries in this bind */
	 bindpkt_params *params; /* first bind params */
}
bindpkt_params_per_process;

/* Note: the following 2 items are used internally via the macro below. */

/* User table type */
typedef struct
{ uint16 delay_flag;  /* 0 means no delay and 1 means with delay */
  uint16 cmd_code;
  word subsysid;
  word count;
  uint16 proc_id;
  const diagpkt_user_table_entry_type *user_table;
} diagpkt_user_table_type;

typedef struct
{
  uint8 command_code;
}
diagpkt_hdr_type;

typedef struct
{
  uint8 command_code;
  uint8 subsys_id;
  uint16 subsys_cmd_code;
}
diagpkt_subsys_hdr_type;

typedef struct
{
  uint8 command_code;
  uint8 subsys_id;
  uint16 subsys_cmd_code;
  uint32 status;  
  uint16 delayed_rsp_id;
  uint16 rsp_cnt; /* 0, means one response and 1, means two responses */
}
diagpkt_subsys_hdr_type_v2;

typedef struct
{
  unsigned int pattern;     /* Pattern to check validity of committed pointers. */
  unsigned int size;        /* Size of usable buffer (diagpkt_q_type->pkt) */
  unsigned int length;      /* Size of packet */

/* LGE_CHANGES_S [kyuhyung.lee@lge.com] 2010-02-08, LG_FW_DIAG_SCREEN_CAPTURE */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_DIAG_SCREEN_CAPTURE) || defined (LG_FW_MTC)
  byte pkt[4096];               /*LG_FW size up*/
#else
  byte pkt[1024];               /* Sized by 'length' field. */
#endif
/* LGE_CHANGES_E [kyuhyung.lee@lge.com] 2010-02-08, LG_FW_DIAG_SCREEN_CAPTURE */
} diagpkt_rsp_type;

typedef void (*diag_cmd_rsp) (const byte *rsp, unsigned int length, void *param);

typedef struct
{
  diag_cmd_rsp rsp_func; /* If !NULL, this is called in lieu of comm layer */
  void *rsp_func_param;

  diagpkt_rsp_type rsp; /* see diagi.h */
} diagpkt_lsm_rsp_type;

typedef struct
{
  uint32 diag_data_type; /* This will be used to identify whether the data passed to DCM is an event, log, F3 or response.*/
  uint8 rest_of_data;
} diag_data;

#define FPOS( type, field ) \
    /*lint -e545 */ ( (dword) &(( type *) 0)-> field ) /*lint +e545 */

#define DIAG_REST_OF_DATA_POS (FPOS(diag_data, rest_of_data))

#define DIAGPKT_DISPATCH_TABLE_REGISTER(xx_subsysid, xx_entry) \
	do { \
		static const diagpkt_user_table_type xx_entry##_table = { \
		 0, 0xFF, xx_subsysid, sizeof (xx_entry) / sizeof (xx_entry[0]), 1, xx_entry \
		}; \
	 /*lint -save -e717 */ \
		diagpkt_tbl_reg (&xx_entry##_table); \
	} while (0)

#define DIAGPKT_PKT2LSMITEM(p) \
		((diagpkt_lsm_rsp_type *) (((byte *) p) - FPOS (diagpkt_lsm_rsp_type, rsp.pkt)))

#endif /* LG_DIAG_KERNEL_SERVICE_H */
