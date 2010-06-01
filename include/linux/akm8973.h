/*
 * Definitions for akm8973 compass chip.
 * this file must same with akm8973.h in AKMD daemon
 */
#ifndef AKM8973_H		/* LGE_CHANGE [hyesung.shin@lge.com] on 2010-1-23, for <Sensor driver structure> */
#define AKM8973_H

#include <linux/ioctl.h>

/* Compass device dependent definition */
#define AKECS_MODE_MEASURE		0x00	/* Starts measurement. */
#define AKECS_MODE_E2P_READ		0x02	/* E2P access mode (read). */
#define AK8973_MS1_READ_EEPROM	0x02
#define AK8973_MS1_WRITE_EEPROM	0xAA
#define AKECS_MODE_POWERDOWN	0x03	/* Power down mode */

/* AK8973 EEPROM address */
#define AK8973_EEP_ETS		0x62
#define AK8973_EEP_EVIR		0x63
#define AK8973_EEP_EIHE		0x64
#define AK8973_EEP_ETST		0x65
#define AK8973_EEP_EHXGA	0x66
#define AK8973_EEP_EHYGA	0x67
#define AK8973_EEP_EHZGA	0x68

#define RBUFF_SIZE			4	/* Rx buffer size */

/* AK8973 register address */
#define AKECS_REG_ST			0xC0
#define AKECS_REG_TMPS			0xC1
#define AKECS_REG_H1X			0xC2
#define AKECS_REG_H1Y			0xC3
#define AKECS_REG_H1Z			0xC4

#define AKECS_REG_MS1			0xE0
#define AKECS_REG_HXDA			0xE1
#define AKECS_REG_HYDA			0xE2
#define AKECS_REG_HZDA			0xE3
#define AKECS_REG_HXGA			0xE4
#define AKECS_REG_HYGA			0xE5
#define AKECS_REG_HZGA			0xE6

#define AKMIO					0xA1

/* IOCTLs for AKM library */
#define ECS_IOCTL_INIT                  _IO(AKMIO, 0x01)
#define ECS_IOCTL_WRITE                 _IOW(AKMIO, 0x02, char[5])
#define ECS_IOCTL_READ                  _IOWR(AKMIO, 0x03, char[5])
#define ECS_IOCTL_RESET      	        _IO(AKMIO, 0x04)
//#define ECS_IOCTL_INT_STATUS            _IO(AKMIO, 0x05)
//#define ECS_IOCTL_FFD_STATUS            _IO(AKMIO, 0x06)
#define ECS_IOCTL_SET_MODE              _IOW(AKMIO, 0x07, short)
#define ECS_IOCTL_GETDATA               _IOR(AKMIO, 0x08, char[RBUFF_SIZE+1])
#define ECS_IOCTL_GET_NUMFRQ            _IOR(AKMIO, 0x09, char[2])
//#define ECS_IOCTL_SET_PERST             _IO(AKMIO, 0x0A)
//#define ECS_IOCTL_SET_G0RST             _IO(AKMIO, 0x0B)
#define ECS_IOCTL_SET_YPR               _IOW(AKMIO, 0x0C, short[12])
#define ECS_IOCTL_GET_OPEN_STATUS       _IOR(AKMIO, 0x0D, int)
#define ECS_IOCTL_GET_CLOSE_STATUS      _IOR(AKMIO, 0x0E, int)
//#define ECS_IOCTL_GET_CALI_DATA         _IOR(AKMIO, 0x0F, char[MAX_CALI_SIZE])
#define ECS_IOCTL_GET_DELAY             _IOR(AKMIO, 0x30, short)

/* IOCTLs for APPs */
//#define ECS_IOCTL_APP_SET_MODE		_IOW(AKMIO, 0x10, short)
#define ECS_IOCTL_APP_SET_MFLAG			_IOW(AKMIO, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG			_IOW(AKMIO, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG			_IOW(AKMIO, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG			_IOR(AKMIO, 0x14, short)
#define ECS_IOCTL_APP_SET_TFLAG			_IOR(AKMIO, 0x15, short)
#define ECS_IOCTL_APP_GET_TFLAG			_IOR(AKMIO, 0x16, short)
//#define ECS_IOCTL_APP_RESET_PEDOMETER   _IO(AKMIO, 0x17)
#define ECS_IOCTL_APP_SET_DELAY			_IOW(AKMIO, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY			ECS_IOCTL_GET_DELAY
#define ECS_IOCTL_APP_SET_MVFLAG		_IOW(AKMIO, 0x19, short)	/* Set raw magnetic vector flag */
#define ECS_IOCTL_APP_GET_MVFLAG		_IOR(AKMIO, 0x1A, short)	/* Get raw magnetic vector flag */
#define ECS_IOCTL_APP_SET_PFLAG			_IOR(AKMIO, 0x1B, short)	/* Get proximity flag */
#define ECS_IOCTL_APP_GET_PFLAG			_IOR(AKMIO, 0x1C, short)	/* Set proximity flag */


/* IOCTLs for pedometer */
//#define ECS_IOCTL_SET_STEP_CNT          _IOW(AKMIO, 0x20, short)

#define ECS_IOCTL_GET_ACCEL_PATH            _IOR(AKMIO, 0x20, char[30])
#define ECS_IOCTL_ACCEL_INIT             _IOR(AKMIO, 0x21, int[7])
#define ECS_IOCTL_GET_ALAYOUT_INIO             _IOR(AKMIO, 0x22, signed short[18])
#define ECS_IOCTL_GET_HLAYOUT_INIO             _IOR(AKMIO, 0x23, signed short[18])

#define AKMD2_TO_ACCEL_IOCTL_READ_XYZ	_IOWR(AKMIO, 0x31, int)
#define AKMD2_TO_ACCEL_IOCTL_ACCEL_INIT	_IOWR(AKMIO, 0x32, int)
#endif	/* AKM8973_H */


