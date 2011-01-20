/*
 *  synaptics_reflash.h :
 */
#ifdef _cplusplus
extern "C" {
#endif

#ifndef _SYNA_REPLASH_LAYER_H
#define _SYNA_REPLSAH_LAYER_H

#include <linux/i2c.h>

void SynaConvertErrCodeToStr(unsigned int errCode, char * errCodeStr, int len);
void SynaCheckIfFatalError(unsigned int errCode);
unsigned int SynaIsExpectedRegFormat(void);
void SynaReadFirmwareInfo(void);
void SynaReadConfigInfo(void);
void SynaSetFlashAddrForDifFormat(void);
void SynaReadPageDescriptionTable(void);
int SynaInitialize(struct i2c_client *syn_touch);
void SynaEnableFlashing(void);
void SynaSpecialCopyEndianAgnostic(unsigned char *dest, unsigned short src) ;
unsigned int SynaReadBootloadID(void);
unsigned int SynaWriteBootloadID(void);
void SynaWaitATTN(int errorCount);
void SynaProgramFirmware(void);
int SynaFlashFirmwareWrite(void);
unsigned int SynaDoReflash(struct i2c_client *syn_touch, int fw_revision);
void RMI4CheckIfFatalError(int errCode);
int SynaFinalizeFlash(void);

#endif /* _SYNA_REPLASH_LAYER_H */

#ifdef _cplusplus
}
#endif

