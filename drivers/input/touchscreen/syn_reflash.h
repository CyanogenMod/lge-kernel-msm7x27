/*
 *  syn_reflash.h :
 */
#ifdef _cplusplus
extern "C" {
#endif

#ifndef _SYNA_REPLASH_LAYER_H
#define _SYNA_REPLSAH_LAYER_H

#include <linux/i2c.h>

enum firmware_type {
	syn_1818,
	syn_1912,
};

int firmware_info(void);
int config_info(void);
int set_flash_addr(void);
int read_description(void);
int enable_flashing(void);
unsigned int SynaReadBootloadID(void);
unsigned int SynaWriteBootloadID(void);
int program_firmware(void);
int flash_fw_write(void);
unsigned int firmware_reflash(struct i2c_client *syn_touch, int fw_revision, enum firmware_type);
int finalize_flash(void);

#endif /* _SYNA_REPLASH_LAYER_H */

#ifdef _cplusplus
}
#endif

