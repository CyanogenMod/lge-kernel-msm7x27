#ifndef LG_DIAG_SCREEN_SHOT_H
#define LG_DIAG_SCREEN_SHOT_H
// LG_FW_DIAG_KERNEL_SERVICE

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
#define LCD_MAIN_WIDTH   320
#define LCD_MAIN_HEIGHT  480

#define SCREEN_SHOT_PACK_LEN  1024 * 2
//#define SCREEN_SHOT_PACK_LEN  100 * 2


#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

typedef enum
{
  SCREEN_SHOT_BK_CTRL = 0,
  SCREEN_SHOT_LCD_BUF,
} diag_screen_shot_sub_cmd_type;


typedef struct
{
  boolean  is_main_lcd;
  word     x;
  word     y;
  word     width;
  word     height;
  dword    total_bytes;
  dword    sended_bytes;
  boolean  update;
  boolean  updated;
  boolean  packed;
  boolean  is_fast_mode;
  boolean  full_draw;
  byte buf[LCD_MAIN_WIDTH * LCD_MAIN_HEIGHT * 2];
}PACKED lcd_buf_info_type;


typedef struct
{
  byte    cmd_code;
  byte    sub_cmd_code;
  boolean main_onoff;
  byte    main_value;
  boolean sub_onoff;
  byte    sub_value;
  boolean ok;
}PACKED diag_lcd_backlight_ctrl_req_type;


typedef enum
{
  SEQ_START = 0,
  SEQ_GET_BUF,
  SEQ_REGET_BUF,
  SEQ_GET_BUF_COMPLETED,
  SEQ_GET_BUF_SUSPEND,
  SEQ_STOP,
} sequence_flow_type;


typedef struct
{
  byte     cmd_code;
  byte     sub_cmd_code;
  boolean  ok;
  boolean  is_main_lcd;
  word     x;
  word     y;
  word     width;
  word     height;
  byte     seq_flow;
  dword    total_bytes;
  dword    sended_bytes;
  boolean  packed;
  boolean  is_fast_mode;
  boolean  full_draw;
  byte     buf[SCREEN_SHOT_PACK_LEN];
}PACKED diag_lcd_get_buf_req_type;


typedef union 
{
	byte cmd_code;
	diag_lcd_backlight_ctrl_req_type lcd_bk_ctrl;
	diag_lcd_get_buf_req_type lcd_buf;
}PACKED diag_screen_shot_type;

#endif /* LG_DIAG_SCREEN_SHOT_H */
