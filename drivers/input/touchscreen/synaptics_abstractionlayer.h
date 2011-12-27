/*
 *  SynaAbstractionLayer.h :
 */
#ifdef _cplusplus
extern "C" {
#endif

#ifndef _SYNA_ABSTRACTION_LAYER_H
#define _SYNA_ABSTRACTION_LAYER_H

#include <linux/i2c.h>

  /* Interfaces to OEM hardware abstraction and O/S abstraction layers.
   * These functions isolate the O/S and H/W specific implimentations of
   * reading, writing and waiting for ATTN on OEMs hardware physical layer.
   *
   * Additionally there are functions that may need to be specifically
   * tailored for the OEMs Operating System - like Sleep, Print and Exit.
   *
   * The OEM customer impliments their own versions of these functions.
   * These are called by the SynaReflash.c code and since they depend on
   * the OEMs specific hardware and O/S they need to be done by the OEM.
   *
   * There is one simple call to make back to the SynaReflash.c code to do the
   * reflashing - this does not need to be changed by the OEM cusomer and is
   * only called once in the main routine.
   */

/* Defined constants  */
#define DefaultErrorRetryCount       300
#define DefaultMillisecondTimeout    300
#define DefaultWaitPeriodAfterReset  200 /* in milliseconds */

/* Special defined error codes used by Synaptics - the OEM should not define any error codes the same. */
#define ESuccess                     0
#define EErrorTimeout               -1
#define EErrorFlashFailed           -2
#define EErrorBootID                -3
#define EErrorFunctionNotSupported  -4
#define EErrorFlashImageCorrupted   -5
#define EErrorNoClient				-6
#define EErrorSMBAddress			-7
#define EErrorSMBlength				-8


void SynaPrintMsg(char *msg);                              /* OEM impliments own print function - this could
                                                            * simply be a call to printf, or some other O/S or system
                                                            * print function that can display the character message
                                                            * on an output device (debug terminal, screen, etc).
                                                            */

void SynaExit(void);                                       /* OEM specifies a version of exit - it may just call exit
                                                            * or some other O/S or system function that terminates and
                                                            * exits the process or terminates the thread.
                                                            */

void SynaSleep(unsigned int MilliSeconds);                /* OEM specific so that we sleep the correct amount. */

int  SynaWaitForATTN(unsigned long MilliSeconds); /* OEM specific - may use different types of syncronization
                                                            * depending on O/S implimentations (signals, events, mutex...).
                                                            * A return value of 0 indicates success - a non-zero return
                                                            * value indicates an error. The specific non-zero return
                                                            * code can be defined by the user. The SynaReglash.c code
                                                            * only checks for 0 (success) or non-zero (failure).
                                                            */

int SynaWriteRegister(unsigned short   uRmiAddress,
                               unsigned char  * data,
                               unsigned int     length);   /* Talks to the H/W to write length bytes from
                                                            * the data buffer to the specified address.
                                                            * A return value of 0 indicates success - a non-zero return
                                                            * value indicates an error. The specific non-zero return
                                                            * code can be defined by the user. The SynaReglash.c code
                                                            * only checks for 0 (success) or non-zero (failure).
                                                            */

int SynaReadRegister(unsigned short  uRmiAddress,
                              unsigned char * data,
                              unsigned int    length);     /* Talks to the H/W to read length bytes from
                                                            * the specified address into the data buffer.
                                                            * A return value of 0 indicates success - a non-zero return
                                                            * value indicates an error. The specific non-zero return
                                                            * code can be defined by the user. The SynaReglash.c code
                                                            * only checks for 0 (success) or non-zero (failure).
                                                            */

void SynaI2CClientInit(struct i2c_client *syn_touch);

#endif /* _SYNA_ABSTRACTION_LAYER_H */

#ifdef _cplusplus
}
#endif
