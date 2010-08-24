#include <linux/module.h>
#include <linux/delay.h>
#include <mach/lg_diagcmd.h>
#include <mach/lg_diag_keypress.h>
#include <linux/input.h>
#include <mach/gpio.h>
/*==========================================================================*/
#define HS_RELEASE_K 0xFFFF
/* 
enum {
	GPIO_SLIDE_CLOSE=0,
	GPIO_SLIDE_OPEN,
};
*/
#define KEY_TRANS_MAP_SIZE 70
typedef struct {
	  word LG_common_key_code;
	    unsigned int Android_key_code;
}keycode_trans_type;

keycode_trans_type keytrans_table[KEY_TRANS_MAP_SIZE]={
    {0x204E        ,   KEY_HOME },
    /* {0x4E        ,   243 },     // folder home for aloha model */
    {0x204F        ,   KEY_MENU },
    /* {0x4F        ,   244 },     // folder menu for aloha model */
    {0x2050        ,   KEY_SEND },
    {0x2051        ,   KEY_END },
    {0x2092        ,   KEY_VOLUMEUP },
    {0x2093        ,   KEY_VOLUMEDOWN},
    {0x8F        ,   KEY_CAMERA },
      
    {0x2023     ,   228}, // pound
    {0x202A     ,   227}, // star
    {0x30     ,   KEY_0},     {0x31     ,   KEY_1},     {0x32     ,   KEY_2},     {0x33     ,   KEY_3},
    {0x34     ,   KEY_4},     {0x35     ,   KEY_5},     {0x36     ,   KEY_6},     {0x37     ,   KEY_7}, 
    {0x38     ,   KEY_8},     {0x39     ,   KEY_9},      

    {0x2030     ,   KEY_0},     {0x2031     ,   KEY_1},     {0x2032     ,   KEY_2},     {0x2033     ,   KEY_3},
    {0x2034     ,   KEY_4},     {0x2035     ,   KEY_5},     {0x2036     ,   KEY_6},     {0x2037     ,   KEY_7}, 
    {0x2038     ,   KEY_8},     {0x2039     ,   KEY_9},      

    {0x2041     ,   KEY_A},  {0x2042     ,   KEY_B}, {0x2043     ,   KEY_C},   {0x2044     ,   KEY_D}, 
    {0x2045     ,   KEY_E},  {0x2046     ,   KEY_F},  {0x2047     ,   KEY_G},   {0x2048     ,   KEY_H}, 
    {0x2049     ,   KEY_I},   {0x204A     ,   KEY_J},  {0x204B     ,   KEY_K},   {0x204C     ,   KEY_L}, 
    {0x204D     ,   KEY_M},  {0x204E     ,   KEY_N}, {0x204F     ,   KEY_O},   {0x2050     ,   KEY_P}, 
    {0x2051     ,   KEY_Q},  {0x2052     ,   KEY_R}, {0x2053     ,   KEY_S},   {0x2054     ,   KEY_T}, 
    {0x2055     ,   KEY_U},  {0x2056     ,   KEY_V}, {0x2057     ,   KEY_W},  {0x2058     ,   KEY_X}, 
    {0x2059     ,   KEY_Y},  {0x205A     ,   KEY_Z}, 

    {0x1010     ,   103},  {0x1011     ,  108},   {0x1054     ,   106},   {0x1055     ,   105}, // left , right, up, down
    {0x1053     ,   KEY_REPLY},   // navi ok
    {0x101D     ,   KEY_ENTER},
    {0x1020     ,   KEY_SPACE},

    {0x1030     ,   KEY_HOME},
    {0x1031     ,   KEY_MENU},
    {0x1032     ,   KEY_BACKSPACE},
    {0x1033     ,   KEY_BACK},
    {0x1034     ,   KEY_SEARCH},
    {0x1035     ,   KEY_LEFTALT},
    {0x1036     ,   KEY_LEFTSHIFT},
    {0x1037     ,   KEY_DOT},
};

unsigned int LGF_KeycodeTrans(word input)
{
  int index = 0;
  unsigned int ret = (unsigned int)input;  // if we can not find, return the org value. 
 
  for( index = 0; index < KEY_TRANS_MAP_SIZE ; index++)
  {
    if( keytrans_table[index].LG_common_key_code == input)
    {
      ret = keytrans_table[index].Android_key_code;
      break;
    }
  }  

  return ret;
}

/* ==========================================================================
===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
//extern unsigned int LGF_KeycodeTrans(word input);
extern void Send_Touch( unsigned int x, unsigned int y);
/*==========================================================================*/

static unsigned saveKeycode =0 ;

void SendKey(unsigned int keycode, unsigned char bHold)
{
  extern struct input_dev *get_ats_input_dev(void);
  struct input_dev *idev = get_ats_input_dev();

  if( keycode != HS_RELEASE_K)
    input_report_key( idev,keycode , 1 ); // press event

  if(bHold)
  {
    saveKeycode = keycode;
  }
  else
  {
    if( keycode != HS_RELEASE_K)
      input_report_key( idev,keycode , 0 ); // release  event
    else
      input_report_key( idev,saveKeycode , 0 ); // release  event
  }
}

#if 0
/*  VS660 don't have HALL IC */
int is_slide_open(void)
{
   if(gpio_get_value(86) == GPIO_SLIDE_OPEN) // hall ic
      return 0;
   else
      return 1;
}
#endif 

PACK (void *)LGF_KeyPress (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
  DIAG_HS_KEY_F_req_type *req_ptr = (DIAG_HS_KEY_F_req_type *) req_pkt_ptr;
  DIAG_HS_KEY_F_rsp_type *rsp_ptr;
  unsigned int keycode = 0;
  const int rsp_len = sizeof( DIAG_HS_KEY_F_rsp_type );

  rsp_ptr = (DIAG_HS_KEY_F_rsp_type *) diagpkt_alloc( DIAG_HS_KEY_F, rsp_len );

  if((req_ptr->magic1 == 0xEA2B7BC0) && (req_ptr->magic2 == 0xA5B7E0DF))
  {
    rsp_ptr->magic1 = req_ptr->magic1;
    rsp_ptr->magic2 = req_ptr->magic2;
    rsp_ptr->key = 0xff; //ignore byte key code
    rsp_ptr->ext_key = req_ptr->ext_key;

    keycode = LGF_KeycodeTrans((word) req_ptr->ext_key);
  }
  else
  {
    rsp_ptr->key = req_ptr->key;
    keycode = LGF_KeycodeTrans((word) req_ptr->key);

  }

  if( keycode == 0xff)
    keycode = HS_RELEASE_K;  // to mach the size
  

  switch (keycode){
	    /* LG_FW 2010.02.24 khlee - UTS TEST needs key to delete call log*/
  	case 0x2060 :
		// touch Dialer start
		Send_Touch(20,478);
		break;
	case 0x2061:
    		//touch call log
      		Send_Touch(100,50);
		break;
	case 0x2062:
    		//touch Clear Call Logs icon
      		Send_Touch(265,470);
		break;
	case 0x2063:
    		//ok  icon
      		Send_Touch(85,315);
		break;
	case 0x2064:
		//power off  button  icon
		Send_Touch(150,350);
		break;
	case 0x2065:
		//power off ok button  icon
		Send_Touch(75,315);
		break;
	case 0x40 :
	    /* LG_FW 2010.02.24 khlee - UTS TEST needs send key to call in idle screen*/
      	   /* FIXME : comment out, taehung.kim@lge.com */
	//	Send_Touch(30,460);

		break;
	default:
    	SendKey(keycode , req_ptr->hold);
		break;
  	}
  	
  return (rsp_ptr);
}

EXPORT_SYMBOL(LGF_KeyPress);
