#ifndef LG_DIAG_WIFI_H
#define LG_DIAG_WIFI_H

#include "lg_comdef.h"
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
#define WCM_MAC_ADDR_LEN 6

typedef struct{
    byte command_code;
    byte sub_cmd;
    byte szMacAddress[WCM_MAC_ADDR_LEN];
}PACKED DIAG_LGE_WIFI_MAC_ADDRESS_req_tag;

typedef struct{
    byte command_code;
    byte sub_cmd;
    byte szMacAddress[WCM_MAC_ADDR_LEN];
    byte result;
}PACKED DIAG_LGE_WIFI_MAC_ADDRESS_rsp_tag;

typedef enum{
    DIAG_WIFI_MAC_ADDRESS_RESULT_OK = 0,                /* Success */
    DIAG_WIFI_MAC_ADDRESS_RESULT_ERROR_NV_READ,         /* NV Read Fail */
    DIAG_WIFI_MAC_ADDRESS_RESULT_ERROR_NV_WRITE,        /* NV Write Fail */
    DIAG_WIFI_MAC_ADDRESS_RESULT_ERROR_NO_PKT,          /* Fail to get a buffer for response */
    DIAG_WIFI_MAC_ADDRESS_RESULT_ERROR_UNKNOWN_CMD,     /* Sub cmd is wrong */
}diag_wifi_mac_address_result_type;

#endif /* LG_DIAG_WIFI_H */
