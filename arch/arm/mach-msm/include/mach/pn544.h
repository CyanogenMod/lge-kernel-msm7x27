/*
 * Copyright (C) 2010 NXP Semiconductors
 */
#define PN544_MAGIC	0xE9

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(2): reset and power on with firmware download enabled
 */
#define PN544_SET_PWR	_IOW(PN544_MAGIC, 0x01, unsigned int)

struct pn544_i2c_platform_data {
  unsigned int sda_gpio; // 2011.02.15 jaejoon.park@lge.com added for LGE structure
  unsigned int scl_gpio; // 2011.02.15 jaejoon.park@lge.com added for LGE structure
	unsigned int irq_gpio;
	unsigned int ven_gpio;
	unsigned int firm_gpio;
};
