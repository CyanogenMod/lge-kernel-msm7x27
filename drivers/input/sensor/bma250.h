/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2010 Bosch Sensortec GmbH
 * All Rights Reserved
 */




/* EasyCASE V6.5 31/03/2010 10:48:14 */
/* EasyCASE O
If=vertical
LevelNumbers=no
LineNumbers=no
Colors=16777215,0,12582912,12632256,0,0,0,16711680,8388736,0,33023,32768,0,0,0,0,0,32768,12632256,255,65280,255,255,16711935
ScreenFont=Courier New,Regular,90,4,-12,0,400,0,0,0,0,0,0,3,2,1,49,96,96
PrinterFont=Courier New,,80,4,-66,0,400,0,0,0,0,0,0,3,2,1,49,600,600
LastLevelId=954 */
/* EasyCASE ( 1
   bma250.h */
#ifndef __BMA250_H__
#define __BMA250_H__
/* EasyCASE - */
/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by ROBERT BOSCH GMBH
 *
 **************************************************************************************************/
/*  $Date: 2010/03/31
 *  $Revision: 1.0 $
 *
 */

/**************************************************************************************************
* Copyright (C) 2007 Bosch Sensortec GmbH
*
* bma250.h
*
* Usage:        BMA250 Sensor Driver Support Header File
*
* Author:       suresh.cp@in.bosch.com
**************************************************************************************************/
/* EasyCASE ( 72
   Disclaimer */
/*************************************************************************************************/
/*  Disclaimer
*
* Common:
* Bosch Sensortec products are developed for the consumer goods industry. They may only be used 
* within the parameters of the respective valid product data sheet.  Bosch Sensortec products are 
* provided with the express understanding that there is no warranty of fitness for a particular purpose. 
* They are not fit for use in life-sustaining, safety or security sensitive systems or any system or device 
* that may lead to bodily harm or property damage if the system or device malfunctions. In addition, 
* Bosch Sensortec products are not fit for use in products which interact with motor vehicle systems.  
* The resale and/or use of products are at the purchaser’s own risk and his own responsibility. The 
* examination of fitness for the intended use is the sole responsibility of the Purchaser. 
*
* The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for 
* incidental, or consequential damages, arising from any product use not covered by the parameters of 
* the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch 
* Sensortec for all costs in connection with such claims.
*
* The purchaser must monitor the market for the purchased products, particularly with regard to 
* product safety and inform Bosch Sensortec without delay of all security relevant incidents.
*
* Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid 
* technical specifications of the product series. They are therefore not intended or fit for resale to third 
* parties or for use in end products. Their sole purpose is internal client testing. The testing of an 
* engineering sample may in no way replace the testing of a product series. Bosch Sensortec 
* assumes no liability for the use of engineering samples. By accepting the engineering samples, the 
* Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering 
* samples.
*
* Special:
* This software module (hereinafter called "Software") and any information on application-sheets 
* (hereinafter called "Information") is provided free of charge for the sole purpose to support your 
* application work. The Software and Information is subject to the following terms and conditions: 
*
* The Software is specifically designed for the exclusive use for Bosch Sensortec products by 
* personnel who have special experience and training. Do not use this Software if you do not have the 
* proper experience or training. 
*
* This Software package is provided `` as is `` and without any expressed or implied warranties, 
* including without limitation, the implied warranties of merchantability and fitness for a particular 
* purpose. 
*
* Bosch Sensortec and their representatives and agents deny any liability for the functional impairment 
* of this Software in terms of fitness, performance and safety. Bosch Sensortec and their 
* representatives and agents shall not be liable for any direct or indirect damages or injury, except as 
* otherwise stipulated in mandatory applicable law.
* 
* The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no 
* responsibility for the consequences of use of such Information nor for any infringement of patents or 
* other rights of third parties which may result from its use. No license is granted by implication or 
* otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are 
* subject to change without notice.
*
* It is not allowed to deliver the source code of the Software to any third party without permission of 
* Bosch Sensortec.
*/
/*************************************************************************************************/
/* EasyCASE ) */
/* EasyCASE ( 913
   File Name For Doxy */
/*! \file bma250.h
    \brief BMA250 Sensor Driver Support Header File */
/* EasyCASE ) */
/* EasyCASE ( 73
   Includes */
/* EasyCASE ( 912
   Standard includes */
/* EasyCASE ) */
/* EasyCASE ( 914
   Module includes */
/* EasyCASE ) */
/* EasyCASE ) */
/* EasyCASE ( 75
   #Define Constants */
/* user defined code to be added here ... */

//Example....
//#define YOUR_H_DEFINE  /**< <Doxy Comment for YOUR_H_DEFINE> */
/* EasyCASE ( 916
   bma250 Macro for read and write commincation */
/** Define the calling convention of YOUR bus communication routine.
        \note This includes types of parameters. This example shows the configuration for an SPI bus link.

    If your communication function looks like this:

    write_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

    The bma250_WR_FUNC_PTR would equal:

    #define     bma250_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char, unsigned char *, unsigned char)

    Parameters can be mixed as needed refer to the \ref bma250_BUS_WRITE_FUNC  macro.


*/
#define bma250_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char *, unsigned char)



/** link makro between API function calls and bus write function
        \note The bus write function can change since this is a system dependant issue.

    If the bus_write parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

    If the parameters are differently ordered or your communication function like I2C need to know the device address,
    you can change this macro accordingly.


    define bma250_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
    bus_write(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the
    \ref bma250_WR_FUNC_PTR definition.



*/
#define bma250_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_write(reg_addr, reg_data, wr_len)


/** Define the calling convention of YOUR bus communication routine.
        \note This includes types of parameters. This example shows the configuration for an SPI bus link.

    If your communication function looks like this:

    read_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

    The bma250_RD_FUNC_PTR would equal:

    #define     bma250_RD_FUNC_PTR char (* bus_read)(unsigned char, unsigned char, unsigned char *, unsigned char)

        Parameters can be mixed as needed refer to the \ref bma250_BUS_READ_FUNC  macro.


*/

#define bma250_SPI_RD_MASK 0x80   /* for spi read transactions on SPI the MSB has to be set */
#define bma250_RD_FUNC_PTR char (* bus_read)( unsigned char, unsigned char *, unsigned char)


/** link makro between API function calls and bus read function
        \note The bus write function can change since this is a system dependant issue.

    If the bus_read parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

    If the parameters are differently ordered or your communication function like I2C need to know the device address,
    you can change this macro accordingly.


        define bma250_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_read(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the
    \ref bma250_WR_FUNC_PTR definition.

        \note: this macro also includes the "MSB='1'" for reading bma250 addresses.

*/
/*
#define bma250_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
           bus_read(reg_addr | bma250_SPI_RD_MASK, reg_data, r_len)


*/
#define bma250_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
           bus_read(reg_addr , reg_data, r_len)
/* EasyCASE ) */
/** bma250 I2C Address
*/

#define BMA250_I2C_ADDR                 0x18

/*
        SMB380 API error codes
*/

#define E_SMB_NULL_PTR          (char)-127
#define E_COMM_RES              (char)-1
#define E_OUT_OF_RANGE          (char)-2
#define E_EEPROM_BUSY           (char)-3

/*
 *
 *      register definitions
 *
 */


#define bma250_EEP_OFFSET                       0x16
#define bma250_IMAGE_BASE                       0x38
#define bma250_IMAGE_LEN                        22


#define bma250_CHIP_ID_REG                      0x00
#define bma250_VERSION_REG                      0x01
#define bma250_X_AXIS_LSB_REG                   0x02
#define bma250_X_AXIS_MSB_REG                   0x03
#define bma250_Y_AXIS_LSB_REG                   0x04
#define bma250_Y_AXIS_MSB_REG                   0x05
#define bma250_Z_AXIS_LSB_REG                   0x06
#define bma250_Z_AXIS_MSB_REG                   0x07
#define bma250_TEMP_RD_REG                      0x08
#define bma250_STATUS1_REG                      0x09
#define bma250_STATUS2_REG                      0x0A
#define bma250_STATUS_TAP_SLOPE_REG             0x0B
#define bma250_STATUS_ORIENT_HIGH_REG           0x0C
#define bma250_RANGE_SEL_REG                    0x0F
#define bma250_BW_SEL_REG                       0x10
#define bma250_MODE_CTRL_REG                    0x11
#define bma250_LOW_NOISE_CTRL_REG               0x12
#define bma250_DATA_CTRL_REG                    0x13
#define bma250_RESET_REG                        0x14
#define bma250_INT_ENABLE1_REG                  0x16
#define bma250_INT_ENABLE2_REG                  0x17
#define bma250_INT1_PAD_SEL_REG                 0x19
#define bma250_INT_DATA_SEL_REG                 0x1A
#define bma250_INT2_PAD_SEL_REG                 0x1B
#define bma250_INT_SRC_REG                      0x1E
#define bma250_INT_SET_REG                      0x20
#define bma250_INT_CTRL_REG                     0x21
#define bma250_LOW_DURN_REG                     0x22
#define bma250_LOW_THRES_REG                    0x23
#define bma250_LOW_HIGH_HYST_REG                0x24
#define bma250_HIGH_DURN_REG                    0x25
#define bma250_HIGH_THRES_REG                   0x26
#define bma250_SLOPE_DURN_REG                   0x27
#define bma250_SLOPE_THRES_REG                  0x28
#define bma250_TAP_PARAM_REG                    0x2A
#define bma250_TAP_THRES_REG                    0x2B
#define bma250_ORIENT_PARAM_REG                 0x2C
#define bma250_THETA_BLOCK_REG                  0x2D
#define bma250_THETA_FLAT_REG                   0x2E
#define bma250_FLAT_HOLD_TIME_REG               0x2F
#define bma250_STATUS_LOW_POWER_REG             0x31
#define bma250_SELF_TEST_REG                    0x32
#define bma250_EEPROM_CTRL_REG                  0x33
#define bma250_SERIAL_CTRL_REG                  0x34
#define bma250_CTRL_UNLOCK_REG                  0x35
#define bma250_OFFSET_CTRL_REG                  0x36
#define bma250_OFFSET_PARAMS_REG                0x37
#define bma250_OFFSET_FILT_X_REG                0x38
#define bma250_OFFSET_FILT_Y_REG                0x39
#define bma250_OFFSET_FILT_Z_REG                0x3A
#define bma250_OFFSET_UNFILT_X_REG              0x3B
#define bma250_OFFSET_UNFILT_Y_REG              0x3C
#define bma250_OFFSET_UNFILT_Z_REG              0x3D
#define bma250_SPARE_0_REG                      0x3E
#define bma250_SPARE_1_REG                      0x3F



/* register write and read delays */

#define bma250_MDELAY_DATA_TYPE                 unsigned int
#define bma250_EE_W_DELAY                       28                    /* delay after EEP write is 28 msec */
/* EasyCASE ( 919
   bma250acc_t */
/* EasyCASE C */
/** bma250 acceleration data
        \brief Structure containing acceleration values for x,y and z-axis in signed short

*/

typedef struct
   {
   short x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
         y, /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
         z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
   } bma250acc_t;
/* EasyCASE E */
/* EasyCASE ) */
/* EasyCASE ( 920
   bma250regs_t */
/* EasyCASE C */
/** bma250 image registers data structure
        \brief Register type that contains all bma250 image registers from address 0x38 to 0x4D
        This structure can hold the complete image data of bma250

*/
typedef struct
   {
   unsigned char
   offset_filt_x ,                 /**<  image address 0x38:  */
   offset_filt_y ,                 /**<  image address 0x39:  */
   offset_filt_z ,                 /**<  image address 0x3A:  */
   offset_unfilt_x ,               /**<  image address 0x3B:  */
   offset_unfilt_y ,               /**<  image address 0x3C:  */
   offset_unfilt_z ,               /**<  image address 0x3D:  */
   spare_0 ,                       /**<  image address 0x3E:  */
   spare_1 ,                       /**<  image address 0x3F:  */
   crc ,                           /**<  image address 0x40:  */
   i2c_addr ,                      /**<  image address 0x41:  */
   dev_config ,                    /**<  image address 0x42:  */
   trim_offset_t ,                 /**<  image address 0x43:  */
   gain_x ,                        /**<  image address 0x44:  */
   offset_x ,                      /**<  image address 0x45:  */
   gain_y ,                        /**<  image address 0x46:  */
   offset_y ,                      /**<  image address 0x47:  */
   gain_z ,                        /**<  image address 0x48:  */
   offset_z ,                      /**<  image address 0x49:  */
   trim1 ,                         /**<  image address 0x4A:  */
   trim2 ,                         /**<  image address 0x4B:  */
   trim3 ,                         /**<  image address 0x4C:  */
   trim4 ;                          /**<  image address 0x4D:  */
   } bma250regs_t;
/* EasyCASE E */
/* EasyCASE ) */
/* EasyCASE ( 921
   bma250_t */
/* EasyCASE C */
/** bma250 typedef structure
        \brief This structure holds all relevant information about bma250 and links communication to the
*/

typedef struct
   {
   bma250regs_t * image;   /**< pointer to bma250regs_t structure not mandatory */
   unsigned char mode;     /**< save current bma250 operation mode */
   unsigned char chip_id,  /**< save bma250's chip id which has to be 0x02 after calling bma250_init() */
                             ml_version, /**< holds the bma250 ML_version number */
                             al_version; /**< holds the bma250 AL_version number */
   unsigned char dev_addr;   /**< initializes bma250's I2C device address 0x38 */
   unsigned char int_mask;   /**< stores the current bma250 API generated interrupt mask */
   bma250_WR_FUNC_PTR;               /**< function pointer to the SPI/I2C write function */
   bma250_RD_FUNC_PTR;               /**< function pointer to the SPI/I2C read function */
   void (*delay_msec)( bma250_MDELAY_DATA_TYPE ); /**< function pointer to a pause in mili seconds function */
   } bma250_t;
/* EasyCASE E */
/* EasyCASE ) */
/* EasyCASE ( 922
   BIT'S & BYTE'S */
#define bma250_CHIP_ID__POS             0
#define bma250_CHIP_ID__MSK             0xFF
#define bma250_CHIP_ID__LEN             8
#define bma250_CHIP_ID__REG             bma250_CHIP_ID_REG

#define bma250_ML_VERSION__POS          0
#define bma250_ML_VERSION__LEN          4
#define bma250_ML_VERSION__MSK          0x0F
#define bma250_ML_VERSION__REG          bma250_VERSION_REG

#define bma250_AL_VERSION__POS          4
#define bma250_AL_VERSION__LEN          4
#define bma250_AL_VERSION__MSK          0xF0
#define bma250_AL_VERSION__REG          bma250_VERSION_REG
/* EasyCASE - */
/* DATA REGISTERS */

#define bma250_NEW_DATA_X__POS          0
#define bma250_NEW_DATA_X__LEN          1
#define bma250_NEW_DATA_X__MSK          0x01
#define bma250_NEW_DATA_X__REG          bma250_X_AXIS_LSB_REG

#define bma250_ACC_X_LSB__POS           6
#define bma250_ACC_X_LSB__LEN           2
#define bma250_ACC_X_LSB__MSK           0xC0
#define bma250_ACC_X_LSB__REG           bma250_X_AXIS_LSB_REG

#define bma250_ACC_X_MSB__POS           0
#define bma250_ACC_X_MSB__LEN           8
#define bma250_ACC_X_MSB__MSK           0xFF
#define bma250_ACC_X_MSB__REG           bma250_X_AXIS_MSB_REG

#define bma250_NEW_DATA_Y__POS          0
#define bma250_NEW_DATA_Y__LEN          1
#define bma250_NEW_DATA_Y__MSK          0x01
#define bma250_NEW_DATA_Y__REG          bma250_Y_AXIS_LSB_REG

#define bma250_ACC_Y_LSB__POS           6
#define bma250_ACC_Y_LSB__LEN           2
#define bma250_ACC_Y_LSB__MSK           0xC0
#define bma250_ACC_Y_LSB__REG           bma250_Y_AXIS_LSB_REG

#define bma250_ACC_Y_MSB__POS           0
#define bma250_ACC_Y_MSB__LEN           8
#define bma250_ACC_Y_MSB__MSK           0xFF
#define bma250_ACC_Y_MSB__REG           bma250_Y_AXIS_MSB_REG

#define bma250_NEW_DATA_Z__POS          0
#define bma250_NEW_DATA_Z__LEN          1
#define bma250_NEW_DATA_Z__MSK          0x01
#define bma250_NEW_DATA_Z__REG          bma250_Z_AXIS_LSB_REG

#define bma250_ACC_Z_LSB__POS           6
#define bma250_ACC_Z_LSB__LEN           2
#define bma250_ACC_Z_LSB__MSK           0xC0
#define bma250_ACC_Z_LSB__REG           bma250_Z_AXIS_LSB_REG

#define bma250_ACC_Z_MSB__POS           0
#define bma250_ACC_Z_MSB__LEN           8
#define bma250_ACC_Z_MSB__MSK           0xFF
#define bma250_ACC_Z_MSB__REG           bma250_Z_AXIS_MSB_REG
/* EasyCASE - */
#define bma250_TEMPERATURE__POS         0
#define bma250_TEMPERATURE__LEN         8
#define bma250_TEMPERATURE__MSK         0xFF
#define bma250_TEMPERATURE__REG         bma250_TEMP_RD_REG
/* EasyCASE - */
/*  INTERRUPT STATUS BITS  */

#define bma250_LOWG_INT_S__POS          0
#define bma250_LOWG_INT_S__LEN          1
#define bma250_LOWG_INT_S__MSK          0x01
#define bma250_LOWG_INT_S__REG          bma250_STATUS1_REG

#define bma250_HIGHG_INT_S__POS          1
#define bma250_HIGHG_INT_S__LEN          1
#define bma250_HIGHG_INT_S__MSK          0x02
#define bma250_HIGHG_INT_S__REG          bma250_STATUS1_REG

#define bma250_SLOPE_INT_S__POS          2
#define bma250_SLOPE_INT_S__LEN          1
#define bma250_SLOPE_INT_S__MSK          0x04
#define bma250_SLOPE_INT_S__REG          bma250_STATUS1_REG

#define bma250_DOUBLE_TAP_INT_S__POS     4
#define bma250_DOUBLE_TAP_INT_S__LEN     1
#define bma250_DOUBLE_TAP_INT_S__MSK     0x10
#define bma250_DOUBLE_TAP_INT_S__REG     bma250_STATUS1_REG

#define bma250_SINGLE_TAP_INT_S__POS     5
#define bma250_SINGLE_TAP_INT_S__LEN     1
#define bma250_SINGLE_TAP_INT_S__MSK     0x20
#define bma250_SINGLE_TAP_INT_S__REG     bma250_STATUS1_REG

#define bma250_ORIENT_INT_S__POS         6
#define bma250_ORIENT_INT_S__LEN         1
#define bma250_ORIENT_INT_S__MSK         0x40
#define bma250_ORIENT_INT_S__REG         bma250_STATUS1_REG

#define bma250_FLAT_INT_S__POS           7
#define bma250_FLAT_INT_S__LEN           1
#define bma250_FLAT_INT_S__MSK           0x80
#define bma250_FLAT_INT_S__REG           bma250_STATUS1_REG

#define bma250_DATA_INT_S__POS           7
#define bma250_DATA_INT_S__LEN           1
#define bma250_DATA_INT_S__MSK           0x80
#define bma250_DATA_INT_S__REG           bma250_STATUS2_REG
/* EasyCASE - */
#define bma250_SLOPE_FIRST_X__POS        0
#define bma250_SLOPE_FIRST_X__LEN        1
#define bma250_SLOPE_FIRST_X__MSK        0x01
#define bma250_SLOPE_FIRST_X__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_SLOPE_FIRST_Y__POS        1
#define bma250_SLOPE_FIRST_Y__LEN        1
#define bma250_SLOPE_FIRST_Y__MSK        0x02
#define bma250_SLOPE_FIRST_Y__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_SLOPE_FIRST_Z__POS        2
#define bma250_SLOPE_FIRST_Z__LEN        1
#define bma250_SLOPE_FIRST_Z__MSK        0x04
#define bma250_SLOPE_FIRST_Z__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_SLOPE_SIGN_S__POS         3
#define bma250_SLOPE_SIGN_S__LEN         1
#define bma250_SLOPE_SIGN_S__MSK         0x08
#define bma250_SLOPE_SIGN_S__REG         bma250_STATUS_TAP_SLOPE_REG
/* EasyCASE - */
#define bma250_TAP_FIRST_X__POS        4
#define bma250_TAP_FIRST_X__LEN        1
#define bma250_TAP_FIRST_X__MSK        0x10
#define bma250_TAP_FIRST_X__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_TAP_FIRST_Y__POS        5
#define bma250_TAP_FIRST_Y__LEN        1
#define bma250_TAP_FIRST_Y__MSK        0x20
#define bma250_TAP_FIRST_Y__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_TAP_FIRST_Z__POS        6
#define bma250_TAP_FIRST_Z__LEN        1
#define bma250_TAP_FIRST_Z__MSK        0x40
#define bma250_TAP_FIRST_Z__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_TAP_FIRST_XYZ__POS        4
#define bma250_TAP_FIRST_XYZ__LEN        3
#define bma250_TAP_FIRST_XYZ__MSK        0x70
#define bma250_TAP_FIRST_XYZ__REG        bma250_STATUS_TAP_SLOPE_REG

#define bma250_TAP_SIGN_S__POS         7
#define bma250_TAP_SIGN_S__LEN         1
#define bma250_TAP_SIGN_S__MSK         0x80
#define bma250_TAP_SIGN_S__REG         bma250_STATUS_TAP_SLOPE_REG
/* EasyCASE - */
#define bma250_HIGHG_FIRST_X__POS        0
#define bma250_HIGHG_FIRST_X__LEN        1
#define bma250_HIGHG_FIRST_X__MSK        0x01
#define bma250_HIGHG_FIRST_X__REG        bma250_STATUS_ORIENT_HIGH_REG

#define bma250_HIGHG_FIRST_Y__POS        1
#define bma250_HIGHG_FIRST_Y__LEN        1
#define bma250_HIGHG_FIRST_Y__MSK        0x02
#define bma250_HIGHG_FIRST_Y__REG        bma250_STATUS_ORIENT_HIGH_REG

#define bma250_HIGHG_FIRST_Z__POS        2
#define bma250_HIGHG_FIRST_Z__LEN        1
#define bma250_HIGHG_FIRST_Z__MSK        0x04
#define bma250_HIGHG_FIRST_Z__REG        bma250_STATUS_ORIENT_HIGH_REG

#define bma250_HIGHG_SIGN_S__POS         3
#define bma250_HIGHG_SIGN_S__LEN         1
#define bma250_HIGHG_SIGN_S__MSK         0x08
#define bma250_HIGHG_SIGN_S__REG         bma250_STATUS_ORIENT_HIGH_REG
/* EasyCASE - */
#define bma250_ORIENT_S__POS             4
#define bma250_ORIENT_S__LEN             3
#define bma250_ORIENT_S__MSK             0x70
#define bma250_ORIENT_S__REG             bma250_STATUS_ORIENT_HIGH_REG

#define bma250_FLAT_S__POS               7
#define bma250_FLAT_S__LEN               1
#define bma250_FLAT_S__MSK               0x80
#define bma250_FLAT_S__REG               bma250_STATUS_ORIENT_HIGH_REG
/* EasyCASE - */
#define bma250_RANGE_SEL__POS             0
#define bma250_RANGE_SEL__LEN             4
#define bma250_RANGE_SEL__MSK             0x0F
#define bma250_RANGE_SEL__REG             bma250_RANGE_SEL_REG
/* EasyCASE - */
#define bma250_BANDWIDTH__POS             0
#define bma250_BANDWIDTH__LEN             5
#define bma250_BANDWIDTH__MSK             0x1F
#define bma250_BANDWIDTH__REG             bma250_BW_SEL_REG
/* EasyCASE - */
#define bma250_SLEEP_DUR__POS             1
#define bma250_SLEEP_DUR__LEN             4
#define bma250_SLEEP_DUR__MSK             0x1E
#define bma250_SLEEP_DUR__REG             bma250_MODE_CTRL_REG
/* EasyCASE - */
#define bma250_EN_LOW_POWER__POS          6
#define bma250_EN_LOW_POWER__LEN          1
#define bma250_EN_LOW_POWER__MSK          0x40
#define bma250_EN_LOW_POWER__REG          bma250_MODE_CTRL_REG
/* EasyCASE - */
#define bma250_EN_SUSPEND__POS            7
#define bma250_EN_SUSPEND__LEN            1
#define bma250_EN_SUSPEND__MSK            0x80
#define bma250_EN_SUSPEND__REG            bma250_MODE_CTRL_REG
/* EasyCASE - */
#define bma250_EN_LOW_NOISE__POS          7
#define bma250_EN_LOW_NOISE__LEN          1
#define bma250_EN_LOW_NOISE__MSK          0x80
#define bma250_EN_LOW_NOISE__REG          bma250_LOW_NOISE_CTRL_REG
/* EasyCASE - */
/**     DISABLE MSB SHADOWING PROCEDURE          **/

#define bma250_DIS_SHADOW_PROC__POS       6
#define bma250_DIS_SHADOW_PROC__LEN       1
#define bma250_DIS_SHADOW_PROC__MSK       0x40
#define bma250_DIS_SHADOW_PROC__REG       bma250_DATA_CTRL_REG

/**     FILTERED OR UNFILTERED ACCELERATION DATA  **/

#define bma250_EN_UNFILT_ACC__POS         7
#define bma250_EN_UNFILT_ACC__LEN         1
#define bma250_EN_UNFILT_ACC__MSK         0x80
#define bma250_EN_UNFILT_ACC__REG         bma250_DATA_CTRL_REG
/* EasyCASE - */
/**     RESET REGISTERS                         **/

#define bma250_EN_SOFT_RESET__POS         0
#define bma250_EN_SOFT_RESET__LEN         8
#define bma250_EN_SOFT_RESET__MSK         0xFF
#define bma250_EN_SOFT_RESET__REG         bma250_RESET_REG

#define bma250_EN_SOFT_RESET_VALUE        0xB6
/* EasyCASE - */
/**     INTERRUPT ENABLE REGISTER              **/


#define bma250_EN_SLOPE_X_INT__POS         0
#define bma250_EN_SLOPE_X_INT__LEN         1
#define bma250_EN_SLOPE_X_INT__MSK         0x01
#define bma250_EN_SLOPE_X_INT__REG         bma250_INT_ENABLE1_REG

#define bma250_EN_SLOPE_Y_INT__POS         1
#define bma250_EN_SLOPE_Y_INT__LEN         1
#define bma250_EN_SLOPE_Y_INT__MSK         0x02
#define bma250_EN_SLOPE_Y_INT__REG         bma250_INT_ENABLE1_REG

#define bma250_EN_SLOPE_Z_INT__POS         2
#define bma250_EN_SLOPE_Z_INT__LEN         1
#define bma250_EN_SLOPE_Z_INT__MSK         0x04
#define bma250_EN_SLOPE_Z_INT__REG         bma250_INT_ENABLE1_REG

#define bma250_EN_SLOPE_XYZ_INT__POS         0
#define bma250_EN_SLOPE_XYZ_INT__LEN         3
#define bma250_EN_SLOPE_XYZ_INT__MSK         0x07
#define bma250_EN_SLOPE_XYZ_INT__REG         bma250_INT_ENABLE1_REG

#define bma250_EN_DOUBLE_TAP_INT__POS      4
#define bma250_EN_DOUBLE_TAP_INT__LEN      1
#define bma250_EN_DOUBLE_TAP_INT__MSK      0x10
#define bma250_EN_DOUBLE_TAP_INT__REG      bma250_INT_ENABLE1_REG

#define bma250_EN_SINGLE_TAP_INT__POS      5
#define bma250_EN_SINGLE_TAP_INT__LEN      1
#define bma250_EN_SINGLE_TAP_INT__MSK      0x20
#define bma250_EN_SINGLE_TAP_INT__REG      bma250_INT_ENABLE1_REG

#define bma250_EN_ORIENT_INT__POS          6
#define bma250_EN_ORIENT_INT__LEN          1
#define bma250_EN_ORIENT_INT__MSK          0x40
#define bma250_EN_ORIENT_INT__REG          bma250_INT_ENABLE1_REG

#define bma250_EN_FLAT_INT__POS            7
#define bma250_EN_FLAT_INT__LEN            1
#define bma250_EN_FLAT_INT__MSK            0x80
#define bma250_EN_FLAT_INT__REG            bma250_INT_ENABLE1_REG
/* EasyCASE - */
/**     INTERRUPT ENABLE REGISTER              **/

#define bma250_EN_HIGHG_X_INT__POS         0
#define bma250_EN_HIGHG_X_INT__LEN         1
#define bma250_EN_HIGHG_X_INT__MSK         0x01
#define bma250_EN_HIGHG_X_INT__REG         bma250_INT_ENABLE2_REG

#define bma250_EN_HIGHG_Y_INT__POS         1
#define bma250_EN_HIGHG_Y_INT__LEN         1
#define bma250_EN_HIGHG_Y_INT__MSK         0x02
#define bma250_EN_HIGHG_Y_INT__REG         bma250_INT_ENABLE2_REG

#define bma250_EN_HIGHG_Z_INT__POS         2
#define bma250_EN_HIGHG_Z_INT__LEN         1
#define bma250_EN_HIGHG_Z_INT__MSK         0x04
#define bma250_EN_HIGHG_Z_INT__REG         bma250_INT_ENABLE2_REG
 
#define bma250_EN_HIGHG_XYZ_INT__POS         2
#define bma250_EN_HIGHG_XYZ_INT__LEN         1
#define bma250_EN_HIGHG_XYZ_INT__MSK         0x04
#define bma250_EN_HIGHG_XYZ_INT__REG         bma250_INT_ENABLE2_REG 

#define bma250_EN_LOWG_INT__POS            3
#define bma250_EN_LOWG_INT__LEN            1
#define bma250_EN_LOWG_INT__MSK            0x08
#define bma250_EN_LOWG_INT__REG            bma250_INT_ENABLE2_REG

#define bma250_EN_NEW_DATA_INT__POS        4
#define bma250_EN_NEW_DATA_INT__LEN        1
#define bma250_EN_NEW_DATA_INT__MSK        0x10
#define bma250_EN_NEW_DATA_INT__REG        bma250_INT_ENABLE2_REG
/* EasyCASE - */
#define bma250_EN_INT1_PAD_LOWG__POS        0
#define bma250_EN_INT1_PAD_LOWG__LEN        1
#define bma250_EN_INT1_PAD_LOWG__MSK        0x01
#define bma250_EN_INT1_PAD_LOWG__REG        bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_HIGHG__POS       1
#define bma250_EN_INT1_PAD_HIGHG__LEN       1
#define bma250_EN_INT1_PAD_HIGHG__MSK       0x02
#define bma250_EN_INT1_PAD_HIGHG__REG       bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_SLOPE__POS       2
#define bma250_EN_INT1_PAD_SLOPE__LEN       1
#define bma250_EN_INT1_PAD_SLOPE__MSK       0x04
#define bma250_EN_INT1_PAD_SLOPE__REG       bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_DB_TAP__POS      4
#define bma250_EN_INT1_PAD_DB_TAP__LEN      1
#define bma250_EN_INT1_PAD_DB_TAP__MSK      0x10
#define bma250_EN_INT1_PAD_DB_TAP__REG      bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_SNG_TAP__POS     5
#define bma250_EN_INT1_PAD_SNG_TAP__LEN     1
#define bma250_EN_INT1_PAD_SNG_TAP__MSK     0x20
#define bma250_EN_INT1_PAD_SNG_TAP__REG     bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_ORIENT__POS      6
#define bma250_EN_INT1_PAD_ORIENT__LEN      1
#define bma250_EN_INT1_PAD_ORIENT__MSK      0x40
#define bma250_EN_INT1_PAD_ORIENT__REG      bma250_INT1_PAD_SEL_REG

#define bma250_EN_INT1_PAD_FLAT__POS        7
#define bma250_EN_INT1_PAD_FLAT__LEN        1
#define bma250_EN_INT1_PAD_FLAT__MSK        0x80
#define bma250_EN_INT1_PAD_FLAT__REG        bma250_INT1_PAD_SEL_REG
/* EasyCASE - */
#define bma250_EN_INT2_PAD_LOWG__POS        0
#define bma250_EN_INT2_PAD_LOWG__LEN        1
#define bma250_EN_INT2_PAD_LOWG__MSK        0x01
#define bma250_EN_INT2_PAD_LOWG__REG        bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_HIGHG__POS       1
#define bma250_EN_INT2_PAD_HIGHG__LEN       1
#define bma250_EN_INT2_PAD_HIGHG__MSK       0x02
#define bma250_EN_INT2_PAD_HIGHG__REG       bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_SLOPE__POS       2
#define bma250_EN_INT2_PAD_SLOPE__LEN       1
#define bma250_EN_INT2_PAD_SLOPE__MSK       0x04
#define bma250_EN_INT2_PAD_SLOPE__REG       bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_DB_TAP__POS      4
#define bma250_EN_INT2_PAD_DB_TAP__LEN      1
#define bma250_EN_INT2_PAD_DB_TAP__MSK      0x10
#define bma250_EN_INT2_PAD_DB_TAP__REG      bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_SNG_TAP__POS     5
#define bma250_EN_INT2_PAD_SNG_TAP__LEN     1
#define bma250_EN_INT2_PAD_SNG_TAP__MSK     0x20
#define bma250_EN_INT2_PAD_SNG_TAP__REG     bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_ORIENT__POS      6
#define bma250_EN_INT2_PAD_ORIENT__LEN      1
#define bma250_EN_INT2_PAD_ORIENT__MSK      0x40
#define bma250_EN_INT2_PAD_ORIENT__REG      bma250_INT2_PAD_SEL_REG

#define bma250_EN_INT2_PAD_FLAT__POS        7
#define bma250_EN_INT2_PAD_FLAT__LEN        1
#define bma250_EN_INT2_PAD_FLAT__MSK        0x80
#define bma250_EN_INT2_PAD_FLAT__REG        bma250_INT2_PAD_SEL_REG
/* EasyCASE - */
#define bma250_EN_INT1_PAD_NEWDATA__POS     0
#define bma250_EN_INT1_PAD_NEWDATA__LEN     1
#define bma250_EN_INT1_PAD_NEWDATA__MSK     0x01
#define bma250_EN_INT1_PAD_NEWDATA__REG     bma250_INT_DATA_SEL_REG

#define bma250_EN_INT2_PAD_NEWDATA__POS     7
#define bma250_EN_INT2_PAD_NEWDATA__LEN     1
#define bma250_EN_INT2_PAD_NEWDATA__MSK     0x80
#define bma250_EN_INT2_PAD_NEWDATA__REG     bma250_INT_DATA_SEL_REG
/* EasyCASE - */
/*****          INTERRUPT SOURCE SELECTION                      *****/

#define bma250_UNFILT_INT_SRC_LOWG__POS        0
#define bma250_UNFILT_INT_SRC_LOWG__LEN        1
#define bma250_UNFILT_INT_SRC_LOWG__MSK        0x01
#define bma250_UNFILT_INT_SRC_LOWG__REG        bma250_INT_SRC_REG

#define bma250_UNFILT_INT_SRC_HIGHG__POS       1
#define bma250_UNFILT_INT_SRC_HIGHG__LEN       1
#define bma250_UNFILT_INT_SRC_HIGHG__MSK       0x02
#define bma250_UNFILT_INT_SRC_HIGHG__REG       bma250_INT_SRC_REG

#define bma250_UNFILT_INT_SRC_SLOPE__POS       2
#define bma250_UNFILT_INT_SRC_SLOPE__LEN       1
#define bma250_UNFILT_INT_SRC_SLOPE__MSK       0x04
#define bma250_UNFILT_INT_SRC_SLOPE__REG       bma250_INT_SRC_REG

#define bma250_UNFILT_INT_SRC_TAP__POS         4
#define bma250_UNFILT_INT_SRC_TAP__LEN         1
#define bma250_UNFILT_INT_SRC_TAP__MSK         0x10
#define bma250_UNFILT_INT_SRC_TAP__REG         bma250_INT_SRC_REG

#define bma250_UNFILT_INT_SRC_DATA__POS        5
#define bma250_UNFILT_INT_SRC_DATA__LEN        1
#define bma250_UNFILT_INT_SRC_DATA__MSK        0x20
#define bma250_UNFILT_INT_SRC_DATA__REG        bma250_INT_SRC_REG
/* EasyCASE - */
/*****  INTERRUPT PAD ACTIVE LEVEL AND OUTPUT TYPE       *****/

#define bma250_INT1_PAD_ACTIVE_LEVEL__POS       0
#define bma250_INT1_PAD_ACTIVE_LEVEL__LEN       1
#define bma250_INT1_PAD_ACTIVE_LEVEL__MSK       0x01
#define bma250_INT1_PAD_ACTIVE_LEVEL__REG       bma250_INT_SET_REG

#define bma250_INT2_PAD_ACTIVE_LEVEL__POS       2
#define bma250_INT2_PAD_ACTIVE_LEVEL__LEN       1
#define bma250_INT2_PAD_ACTIVE_LEVEL__MSK       0x04
#define bma250_INT2_PAD_ACTIVE_LEVEL__REG       bma250_INT_SET_REG


/*****  OUTPUT TYPE IF SET TO 1 IS : OPEN DRIVE , IF NOT SET
        IT IS PUSH-PULL                                  *****/


#define bma250_INT1_PAD_OUTPUT_TYPE__POS        1
#define bma250_INT1_PAD_OUTPUT_TYPE__LEN        1
#define bma250_INT1_PAD_OUTPUT_TYPE__MSK        0x02
#define bma250_INT1_PAD_OUTPUT_TYPE__REG        bma250_INT_SET_REG

#define bma250_INT2_PAD_OUTPUT_TYPE__POS        3
#define bma250_INT2_PAD_OUTPUT_TYPE__LEN        1
#define bma250_INT2_PAD_OUTPUT_TYPE__MSK        0x08
#define bma250_INT2_PAD_OUTPUT_TYPE__REG        bma250_INT_SET_REG
/* EasyCASE - */
/*****               INTERRUPT MODE SELECTION              ******/

#define bma250_INT_MODE_SEL__POS                0
#define bma250_INT_MODE_SEL__LEN                4
#define bma250_INT_MODE_SEL__MSK                0x0F
#define bma250_INT_MODE_SEL__REG                bma250_INT_CTRL_REG

/*****               LATCHED INTERRUPT RESET               ******/

#define bma250_INT_RESET_LATCHED__POS           7
#define bma250_INT_RESET_LATCHED__LEN           1
#define bma250_INT_RESET_LATCHED__MSK           0x80
#define bma250_INT_RESET_LATCHED__REG           bma250_INT_CTRL_REG
/* EasyCASE - */
/*****               LOW-G DURATION                        ******/

#define bma250_LOWG_DUR__POS                    0
#define bma250_LOWG_DUR__LEN                    8
#define bma250_LOWG_DUR__MSK                    0xFF
#define bma250_LOWG_DUR__REG                    bma250_LOW_DURN_REG

/*****               LOW-G THRESHOLD                       ******/

#define bma250_LOWG_THRES__POS                  0
#define bma250_LOWG_THRES__LEN                  8
#define bma250_LOWG_THRES__MSK                  0xFF
#define bma250_LOWG_THRES__REG                  bma250_LOW_THRES_REG

/*****               LOW-G HYSTERESIS                       ******/

#define bma250_LOWG_HYST__POS                   0
#define bma250_LOWG_HYST__LEN                   2
#define bma250_LOWG_HYST__MSK                   0x03
#define bma250_LOWG_HYST__REG                   bma250_LOW_HIGH_HYST_REG

/*****               LOW-G INTERRUPT MODE                   ******/
/*****       IF 1 -- SUM MODE , 0 -- SINGLE MODE            ******/
#define bma250_LOWG_INT_MODE__POS               2
#define bma250_LOWG_INT_MODE__LEN               1
#define bma250_LOWG_INT_MODE__MSK               0x04
#define bma250_LOWG_INT_MODE__REG               bma250_LOW_HIGH_HYST_REG
/* EasyCASE - */
/*****               HIGH-G DURATION                        ******/

#define bma250_HIGHG_DUR__POS                    0
#define bma250_HIGHG_DUR__LEN                    8
#define bma250_HIGHG_DUR__MSK                    0xFF
#define bma250_HIGHG_DUR__REG                    bma250_HIGH_DURN_REG

/*****               HIGH-G THRESHOLD                       ******/

#define bma250_HIGHG_THRES__POS                  0
#define bma250_HIGHG_THRES__LEN                  8
#define bma250_HIGHG_THRES__MSK                  0xFF
#define bma250_HIGHG_THRES__REG                  bma250_HIGH_THRES_REG


/*****               HIGH-G HYSTERESIS                       ******/

#define bma250_HIGHG_HYST__POS                  6
#define bma250_HIGHG_HYST__LEN                  2
#define bma250_HIGHG_HYST__MSK                  0xC0
#define bma250_HIGHG_HYST__REG                  bma250_LOW_HIGH_HYST_REG
/* EasyCASE - */
/*****               SLOPE DURATION                        ******/

#define bma250_SLOPE_DUR__POS                    0
#define bma250_SLOPE_DUR__LEN                    2
#define bma250_SLOPE_DUR__MSK                    0x03
#define bma250_SLOPE_DUR__REG                    bma250_SLOPE_DURN_REG
/* EasyCASE - */
/*****               SLOPE THRESHOLD                       ******/

#define bma250_SLOPE_THRES__POS                  0
#define bma250_SLOPE_THRES__LEN                  8
#define bma250_SLOPE_THRES__MSK                  0xFF
#define bma250_SLOPE_THRES__REG                  bma250_SLOPE_THRES_REG
/* EasyCASE - */
/*****               TAP DURATION                        ******/

#define bma250_TAP_DUR__POS                    0
#define bma250_TAP_DUR__LEN                    3
#define bma250_TAP_DUR__MSK                    0x07
#define bma250_TAP_DUR__REG                    bma250_TAP_PARAM_REG

/*****               TAP SHOCK DURATION                 ******/

#define bma250_TAP_SHOCK_DURN__POS             6
#define bma250_TAP_SHOCK_DURN__LEN             1
#define bma250_TAP_SHOCK_DURN__MSK             0x40
#define bma250_TAP_SHOCK_DURN__REG             bma250_TAP_PARAM_REG

/*****               TAP QUIET DURATION                 ******/

#define bma250_TAP_QUIET_DURN__POS             7
#define bma250_TAP_QUIET_DURN__LEN             1
#define bma250_TAP_QUIET_DURN__MSK             0x80
#define bma250_TAP_QUIET_DURN__REG             bma250_TAP_PARAM_REG
/* EasyCASE - */
/*****               TAP THRESHOLD                       ******/

#define bma250_TAP_THRES__POS                  0
#define bma250_TAP_THRES__LEN                  5
#define bma250_TAP_THRES__MSK                  0x1F
#define bma250_TAP_THRES__REG                  bma250_TAP_THRES_REG

/*****               TAP SAMPLES                         ******/

#define bma250_TAP_SAMPLES__POS                6
#define bma250_TAP_SAMPLES__LEN                2
#define bma250_TAP_SAMPLES__MSK                0xC0
#define bma250_TAP_SAMPLES__REG                bma250_TAP_THRES_REG
/* EasyCASE - */
/*****       ORIENTATION MODE                        ******/

#define bma250_ORIENT_MODE__POS                  0
#define bma250_ORIENT_MODE__LEN                  2
#define bma250_ORIENT_MODE__MSK                  0x03
#define bma250_ORIENT_MODE__REG                  bma250_ORIENT_PARAM_REG

/*****       ORIENTATION BLOCKING                    ******/

#define bma250_ORIENT_BLOCK__POS                 2
#define bma250_ORIENT_BLOCK__LEN                 2
#define bma250_ORIENT_BLOCK__MSK                 0x0C
#define bma250_ORIENT_BLOCK__REG                 bma250_ORIENT_PARAM_REG

/*****       ORIENTATION HYSTERESIS                  ******/

#define bma250_ORIENT_HYST__POS                  4
#define bma250_ORIENT_HYST__LEN                  3
#define bma250_ORIENT_HYST__MSK                  0x70
#define bma250_ORIENT_HYST__REG                  bma250_ORIENT_PARAM_REG
/* EasyCASE - */
/*****       ORIENTATION AXIS SELECTION              ******/
/***** IF SET TO 1 -- X AND Z ARE SWAPPED , Y IS INVERTED */

#define bma250_ORIENT_AXIS__POS                  7
#define bma250_ORIENT_AXIS__LEN                  1
#define bma250_ORIENT_AXIS__MSK                  0x80
#define bma250_ORIENT_AXIS__REG                  bma250_THETA_BLOCK_REG

/*****       THETA BLOCKING                    ******/

#define bma250_THETA_BLOCK__POS                  0
#define bma250_THETA_BLOCK__LEN                  6
#define bma250_THETA_BLOCK__MSK                  0x3F
#define bma250_THETA_BLOCK__REG                  bma250_THETA_BLOCK_REG
/* EasyCASE - */
/*****       THETA FLAT                        ******/

#define bma250_THETA_FLAT__POS                  0
#define bma250_THETA_FLAT__LEN                  6
#define bma250_THETA_FLAT__MSK                  0x3F
#define bma250_THETA_FLAT__REG                  bma250_THETA_FLAT_REG
/* EasyCASE - */
/*****      FLAT HOLD TIME                     ******/

#define bma250_FLAT_HOLD_TIME__POS              4
#define bma250_FLAT_HOLD_TIME__LEN              2
#define bma250_FLAT_HOLD_TIME__MSK              0x30
#define bma250_FLAT_HOLD_TIME__REG              bma250_FLAT_HOLD_TIME_REG
/* EasyCASE - */
/*****      LOW POWER MODE -STATUS             ******/

#define bma250_LOW_POWER_MODE_S__POS            0
#define bma250_LOW_POWER_MODE_S__LEN            1
#define bma250_LOW_POWER_MODE_S__MSK            0x01
#define bma250_LOW_POWER_MODE_S__REG            bma250_STATUS_LOW_POWER_REG
/* EasyCASE - */
/*****      ACTIVATE SELF TEST                 ******/

#define bma250_EN_SELF_TEST__POS                0
#define bma250_EN_SELF_TEST__LEN                2
#define bma250_EN_SELF_TEST__MSK                0x03
#define bma250_EN_SELF_TEST__REG                bma250_SELF_TEST_REG

/*****     SELF TEST -- NEGATIVE               ******/

#define bma250_NEG_SELF_TEST__POS               2
#define bma250_NEG_SELF_TEST__LEN               1
#define bma250_NEG_SELF_TEST__MSK               0x04
#define bma250_NEG_SELF_TEST__REG               bma250_SELF_TEST_REG

/*****     SELF TEST AMPLITUDE                 ******/

#define bma250_SELF_TEST_AMP__POS               4
#define bma250_SELF_TEST_AMP__LEN               3
#define bma250_SELF_TEST_AMP__MSK               0x70
#define bma250_SELF_TEST_AMP__REG               bma250_SELF_TEST_REG
/* EasyCASE - */
/*****     EEPROM CONTROL                      ******/

/* SETTING THIS BIT  UNLOCK'S WRITING SETTING REGISTERS TO EEPROM */

#define bma250_UNLOCK_EE_WRITE_SETTING__POS     0
#define bma250_UNLOCK_EE_WRITE_SETTING__LEN     1
#define bma250_UNLOCK_EE_WRITE_SETTING__MSK     0x01
#define bma250_UNLOCK_EE_WRITE_SETTING__REG     bma250_EEPROM_CTRL_REG


/* SETTING THIS BIT STARTS WRITING SETTING REGISTERS TO EEPROM */

#define bma250_START_EE_WRITE_SETTING__POS      1
#define bma250_START_EE_WRITE_SETTING__LEN      1
#define bma250_START_EE_WRITE_SETTING__MSK      0x02
#define bma250_START_EE_WRITE_SETTING__REG      bma250_EEPROM_CTRL_REG


/* STATUS OF WRITING TO EEPROM */

#define bma250_EE_WRITE_SETTING_S__POS          2
#define bma250_EE_WRITE_SETTING_S__LEN          1
#define bma250_EE_WRITE_SETTING_S__MSK          0x04
#define bma250_EE_WRITE_SETTING_S__REG          bma250_EEPROM_CTRL_REG


/* UPDATE IMAGE REGISTERS WRITING TO EEPROM */

#define bma250_UPDATE_IMAGE__POS                3
#define bma250_UPDATE_IMAGE__LEN                1
#define bma250_UPDATE_IMAGE__MSK                0x08
#define bma250_UPDATE_IMAGE__REG                bma250_EEPROM_CTRL_REG


/* STATUS OF IMAGE REGISTERS WRITING TO EEPROM */

#define bma250_IMAGE_REG_EE_WRITE_S__POS        3
#define bma250_IMAGE_REG_EE_WRITE_S__LEN        1
#define bma250_IMAGE_REG_EE_WRITE_S__MSK        0x08
#define bma250_IMAGE_REG_EE_WRITE_S__REG        bma250_EEPROM_CTRL_REG
/* EasyCASE - */
/* SPI INTERFACE MODE SELECTION */

#define bma250_EN_SPI_MODE_3__POS              0
#define bma250_EN_SPI_MODE_3__LEN              1
#define bma250_EN_SPI_MODE_3__MSK              0x01
#define bma250_EN_SPI_MODE_3__REG              bma250_SERIAL_CTRL_REG

/* I2C WATCHDOG PERIOD SELECTION */

#define bma250_I2C_WATCHDOG_PERIOD__POS        1
#define bma250_I2C_WATCHDOG_PERIOD__LEN        1
#define bma250_I2C_WATCHDOG_PERIOD__MSK        0x02
#define bma250_I2C_WATCHDOG_PERIOD__REG        bma250_SERIAL_CTRL_REG

/* I2C WATCHDOG SELECTION */

#define bma250_EN_I2C_WATCHDOG__POS            2
#define bma250_EN_I2C_WATCHDOG__LEN            1
#define bma250_EN_I2C_WATCHDOG__MSK            0x04
#define bma250_EN_I2C_WATCHDOG__REG            bma250_SERIAL_CTRL_REG
/* EasyCASE - */
/* SETTING THIS BIT  UNLOCK'S WRITING TRIMMING REGISTERS TO EEPROM */

#define bma250_UNLOCK_EE_WRITE_TRIM__POS        4
#define bma250_UNLOCK_EE_WRITE_TRIM__LEN        4
#define bma250_UNLOCK_EE_WRITE_TRIM__MSK        0xF0
#define bma250_UNLOCK_EE_WRITE_TRIM__REG        bma250_CTRL_UNLOCK_REG
/* EasyCASE - */
/**    OFFSET  COMPENSATION     **/

/**    SLOW COMPENSATION FOR X,Y,Z AXIS      **/

#define bma250_EN_SLOW_COMP_X__POS              0
#define bma250_EN_SLOW_COMP_X__LEN              1
#define bma250_EN_SLOW_COMP_X__MSK              0x01
#define bma250_EN_SLOW_COMP_X__REG              bma250_OFFSET_CTRL_REG

#define bma250_EN_SLOW_COMP_Y__POS              1
#define bma250_EN_SLOW_COMP_Y__LEN              1
#define bma250_EN_SLOW_COMP_Y__MSK              0x02
#define bma250_EN_SLOW_COMP_Y__REG              bma250_OFFSET_CTRL_REG

#define bma250_EN_SLOW_COMP_Z__POS              2
#define bma250_EN_SLOW_COMP_Z__LEN              1
#define bma250_EN_SLOW_COMP_Z__MSK              0x04
#define bma250_EN_SLOW_COMP_Z__REG              bma250_OFFSET_CTRL_REG

#define bma250_EN_SLOW_COMP_XYZ__POS              0
#define bma250_EN_SLOW_COMP_XYZ__LEN              3
#define bma250_EN_SLOW_COMP_XYZ__MSK              0x07
#define bma250_EN_SLOW_COMP_XYZ__REG              bma250_OFFSET_CTRL_REG

/**    FAST COMPENSATION READY FLAG          **/

#define bma250_FAST_COMP_RDY_S__POS             4
#define bma250_FAST_COMP_RDY_S__LEN             1
#define bma250_FAST_COMP_RDY_S__MSK             0x10
#define bma250_FAST_COMP_RDY_S__REG             bma250_OFFSET_CTRL_REG

/**    FAST COMPENSATION FOR X,Y,Z AXIS      **/

#define bma250_EN_FAST_COMP__POS                5
#define bma250_EN_FAST_COMP__LEN                2
#define bma250_EN_FAST_COMP__MSK                0x60
#define bma250_EN_FAST_COMP__REG                bma250_OFFSET_CTRL_REG

/**    RESET OFFSET REGISTERS                **/

#define bma250_RESET_OFFSET_REGS__POS           7
#define bma250_RESET_OFFSET_REGS__LEN           1
#define bma250_RESET_OFFSET_REGS__MSK           0x80
#define bma250_RESET_OFFSET_REGS__REG           bma250_OFFSET_CTRL_REG
/* EasyCASE - */
/**     SLOW COMPENSATION  CUTOFF               **/

#define bma250_COMP_CUTOFF__POS                 0
#define bma250_COMP_CUTOFF__LEN                 1
#define bma250_COMP_CUTOFF__MSK                 0x01
#define bma250_COMP_CUTOFF__REG                 bma250_OFFSET_PARAMS_REG

/**     COMPENSATION TARGET                  **/

#define bma250_COMP_TARGET_OFFSET_X__POS        1
#define bma250_COMP_TARGET_OFFSET_X__LEN        2
#define bma250_COMP_TARGET_OFFSET_X__MSK        0x06
#define bma250_COMP_TARGET_OFFSET_X__REG        bma250_OFFSET_PARAMS_REG

#define bma250_COMP_TARGET_OFFSET_Y__POS        3
#define bma250_COMP_TARGET_OFFSET_Y__LEN        2
#define bma250_COMP_TARGET_OFFSET_Y__MSK        0x18
#define bma250_COMP_TARGET_OFFSET_Y__REG        bma250_OFFSET_PARAMS_REG

#define bma250_COMP_TARGET_OFFSET_Z__POS        5
#define bma250_COMP_TARGET_OFFSET_Z__LEN        2
#define bma250_COMP_TARGET_OFFSET_Z__MSK        0x60
#define bma250_COMP_TARGET_OFFSET_Z__REG        bma250_OFFSET_PARAMS_REG
/* EasyCASE ) */
#define bma250_GET_BITSLICE(regvar, bitname)\
                        (regvar & bitname##__MSK) >> bitname##__POS


#define bma250_SET_BITSLICE(regvar, bitname, val)\
                  (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)


/** \endcond */


/* CONSTANTS */


/* range and bandwidth */

#define bma250_RANGE_2G                 0 /**< sets range to +/- 2G mode \see bma250_set_range() */
#define bma250_RANGE_4G                 1 /**< sets range to +/- 4G mode \see bma250_set_range() */
#define bma250_RANGE_8G                 2 /**< sets range to +/- 8G mode \see bma250_set_range() */
#define bma250_RANGE_16G                3 /**< sets range to +/- 16G mode \see bma250_set_range() */


#define bma250_BW_7_81HZ        0x08       /**< sets bandwidth to LowPass 7.81  HZ \see bma250_set_bandwidth() */
#define bma250_BW_15_63HZ       0x09       /**< sets bandwidth to LowPass 15.63 HZ \see bma250_set_bandwidth() */
#define bma250_BW_31_25HZ       0x0A       /**< sets bandwidth to LowPass 31.25 HZ \see bma250_set_bandwidth() */
#define bma250_BW_62_50HZ       0x0B       /**< sets bandwidth to LowPass 62.50 HZ \see bma250_set_bandwidth() */
#define bma250_BW_125HZ         0x0C       /**< sets bandwidth to LowPass 125HZ \see bma250_set_bandwidth() */
#define bma250_BW_250HZ         0x0D       /**< sets bandwidth to LowPass 250HZ \see bma250_set_bandwidth() */
#define bma250_BW_500HZ         0x0E       /**< sets bandwidth to LowPass 500HZ \see bma250_set_bandwidth() */
#define bma250_BW_1000HZ        0x0F       /**< sets bandwidth to LowPass 1000HZ \see bma250_set_bandwidth() */

/* mode settings */

#define bma250_MODE_NORMAL      0
#define bma250_MODE_LOWPOWER    1
#define bma250_MODE_SUSPEND     2

/* wake up */

#define bma250_WAKE_UP_DUR_20MS         0
#define bma250_WAKE_UP_DUR_80MS         1
#define bma250_WAKE_UP_DUR_320MS                2
#define bma250_WAKE_UP_DUR_2560MS               3


/* LG/HG thresholds are in LSB and depend on RANGE setting */
/* no range check on threshold calculation */

#define bma250_SELF_TEST0_ON            1
#define bma250_SELF_TEST1_ON            2

#define bma250_EE_W_OFF                 0
#define bma250_EE_W_ON                  1
/* EasyCASE ( 925
   MACRO's to convert g values to register values */
/** Macro to convert floating point low-g-thresholds in G to 8-bit register values.<br>
  * Example: bma250_LOW_TH_IN_G( 0.3, 2.0) generates the register value for 0.3G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define bma250_LOW_TH_IN_G( gthres, range)                      ((256 * gthres ) / range)

/** Macro to convert floating point high-g-thresholds in G to 8-bit register values.<br>
  * Example: bma250_HIGH_TH_IN_G( 1.4, 2.0) generates the register value for 1.4G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define bma250_HIGH_TH_IN_G(gthres, range)                              ((256 * gthres ) / range)

/** Macro to convert floating point low-g-hysteresis in G to 8-bit register values.<br>
  * Example: bma250_LOW_HY_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define bma250_LOW_HY_IN_G( ghyst, range )                              ((32 * ghyst) / range)

/** Macro to convert floating point high-g-hysteresis in G to 8-bit register values.<br>
  * Example: bma250_HIGH_HY_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define bma250_HIGH_HY_IN_G( ghyst, range )                             ((32 * ghyst) / range)


/** Macro to convert floating point G-thresholds to 8-bit register values<br>
  * Example: bma250_SLOPE_TH_IN_G( 1.2, 2.0) generates the register value for 1.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */

#define bma250_SLOPE_TH_IN_G( gthres, range)    ((128 * gthres ) / range)
/* EasyCASE ) */
/* EasyCASE ) */
/* EasyCASE ( 76
   ENUM and struct Definitions */
/*user defined Enums*/
/* EasyCASE - */
/* EasyCASE < */
//Example..
//enum {
//E_YOURDATA1, /**< <DOXY Comment for E_YOURDATA1> */
//E_YOURDATA2  /**< <DOXY Comment for E_YOURDATA2> */
//};
/* EasyCASE > */
/* EasyCASE - */
/*user defined Structures*/
/* EasyCASE - */
/* EasyCASE < */
//Example...
//struct DUMMY_STRUCT {
//data1, /**< <DOXY Comment for data1> */
//data2  /**< <DOXY Comment for data1> */
//};
/* EasyCASE > */
/* EasyCASE ) */
/* EasyCASE ( 79
      Public API Declarations */
/* EasyCASE ( 927
   bma250_soft_reset */

int bma250_soft_reset(void);
/* EasyCASE ) */
/* EasyCASE ( 928
   bma250_init */
int bma250_init(bma250_t *bma250) ;
/* EasyCASE ) */
/* EasyCASE ( 929
   bma250_set_MEMS */
int bma250_set_MEMS(unsigned char CMB381 , unsigned char Noise_level );
/* EasyCASE ) */
/* EasyCASE ( 930
   bma250_set_LowNoise */
int bma250_set_LowNoise (unsigned char Noise_Mode)         ;
/* EasyCASE ) */
/* EasyCASE ( 932
   bma250_set_mode */
unsigned char bma250_set_mode(unsigned char Mode);
/* EasyCASE ) */
/* EasyCASE ( 933
   bma250_get_mode */
int bma250_get_mode(unsigned char * Mode );
/* EasyCASE ) */
/* EasyCASE ( 934
   bma250_set_range */
unsigned char bma250_set_range(unsigned char Range);
/* EasyCASE ) */
/* EasyCASE ( 935
   bma250_get_range */
int bma250_get_range(unsigned char * Range );
/* EasyCASE ) */
/* EasyCASE ( 936
   bma250_set_bandwidth */
int bma250_set_bandwidth(unsigned char bw);
/* EasyCASE ) */
/* EasyCASE ( 937
   bma250_get_bandwidth */
int bma250_get_bandwidth(unsigned char * bw);
/* EasyCASE ) */
/* EasyCASE ( 938
   bma250_write_reg */
int bma250_write_reg(unsigned char addr, unsigned char *data, unsigned char len);
/* EasyCASE ) */
/* EasyCASE ( 939
   bma250_read_reg */
int bma250_read_reg(unsigned char addr, unsigned char *data, unsigned char len);
/* EasyCASE ) */
/* EasyCASE ( 940
   bma250_reset_interrupt */
int bma250_reset_interrupt(void);
/* EasyCASE ) */
/* EasyCASE ( 941
   bma250_read_accel_xyz */
int bma250_read_accel_xyz(bma250acc_t * acc);
/* EasyCASE ) */
/* EasyCASE ( 942
   bma250_read_accel_x */
int bma250_read_accel_x(short *a_x);
/* EasyCASE ) */
/* EasyCASE ( 943
   bma250_read_accel_y */
int bma250_read_accel_y(short *a_y);
/* EasyCASE ) */
/* EasyCASE ( 944
   bma250_read_accel_z */
int bma250_read_accel_z(short *a_z);
/* EasyCASE ) */
/* EasyCASE ( 964
   bma250_get_interruptstatus1 */
int bma250_get_interruptstatus1(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 966
   bma250_get_interruptstatus2 */
int bma250_get_interruptstatus2(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 974
   bma250_get_Low_G_interrupt */
int bma250_get_Low_G_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 978
   bma250_get_High_G_Interrupt */
int bma250_get_High_G_Interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 979
   bma250_get_slope_interrupt */
int bma250_get_slope_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 980
   bma250_get_double_tap_interrupt */
int bma250_get_double_tap_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 981
   bma250_get_single_tap_interrupt */
int bma250_get_single_tap_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 983
   bma250_get_orient_interrupt */
int bma250_get_orient_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 984
   bma250_get_flat_interrupt */
int bma250_get_flat_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 985
   bma250_get_data_interrupt */
int bma250_get_data_interrupt(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 986
   bma250_get_slope_first */
int bma250_get_slope_first(unsigned char param,unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 988
   bma250_get_slope_sign */
int bma250_get_slope_sign(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 989
   bma250_get_tap_first */
int bma250_get_tap_first(unsigned char param,unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 990
   bma250_get_tap_sign */
int bma250_get_tap_sign(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 991
   bma250_get_HIGH_first */
int bma250_get_HIGH_first(unsigned char param,unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 995
   bma250_get_HIGH_sign */
int bma250_get_HIGH_sign(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 996
   bma250_get_orient_status */
int bma250_get_orient_status(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 997
   bma250_get_orient_flat_status */
int bma250_get_orient_flat_status(unsigned char *intstatus );
/* EasyCASE ) */
/* EasyCASE ( 998
   bma250_get_sleep_duration */
int bma250_get_sleep_duration(unsigned char *sleep );
/* EasyCASE ) */
/* EasyCASE ( 1000
   bma250_set_sleep_duration */
int bma250_set_sleep_duration(unsigned char sleepdur );
/* EasyCASE ) */
/* EasyCASE ( 1001
   bma250_set_suspend */
int bma250_set_suspend(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1002
   bma250_get_suspend */
int bma250_get_suspend(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1003
   bma250_set_lowpower */
int bma250_set_lowpower(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1007
   bma250_get_lowpower_en */
int bma250_get_lowpower_en(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1008
   bma250_set_low_noise_ctrl */
int bma250_set_low_noise_ctrl(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1009
   bma250_get_low_noise_ctrl */
int bma250_get_low_noise_ctrl(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1011
   bma250_set_shadow_disable */
int bma250_set_shadow_disable(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1012
   bma250_get_shadow_disable */
int bma250_get_shadow_disable(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1013
   bma250_set_unfilt_acc */
int bma250_set_unfilt_acc(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1015
   bma250_get_unfilt_acc */
int bma250_get_unfilt_acc(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1016
   bma250_set_enable_slope_interrupt */
int bma250_set_enable_slope_interrupt(unsigned char slope);
/* EasyCASE ) */
/* EasyCASE ( 1017
   bma250_get_enable_slope_interrupt */
int bma250_get_enable_slope_interrupt(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1022
   bma250_get_enable_tap_interrupt */
int bma250_set_enable_tap_interrupt(unsigned char tapinterrupt);
int bma250_get_enable_tap_interrupt(unsigned char param,unsigned char *status );

/* EasyCASE ) */
/* EasyCASE ( 1023
   bma250_set_enable_high_g_interrupt */
int bma250_set_enable_high_g_interrupt(unsigned char highinterrupt);
/* EasyCASE ) */
/* EasyCASE ( 1025
   bma250_get_enable_high_g_interrupt */
int bma250_get_enable_high_g_interrupt(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1026
   bma250_set_enable_low_g_interrupt */
int bma250_set_enable_low_g_interrupt(void);
/* EasyCASE ) */
/* EasyCASE ( 1027
   bma250_get_enable_low_g_interrupt */
int bma250_get_enable_low_g_interrupt(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1029
   bma250_set_enable_data_interrupt */
int bma250_set_enable_data_interrupt(void);
/* EasyCASE ) */
/* EasyCASE ( 1030
   bma250_get_enable_data_interrupt */
int bma250_get_enable_data_interrupt(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1031
   bma250_set_int1_pad_sel */
int bma250_set_int1_pad_sel(unsigned char int1sel);
/* EasyCASE ) */
/* EasyCASE ( 1035
   bma250_get_int1_pad_sel */
int bma250_get_int1_pad_sel(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1036
   bma250_set_int_data_sel */
int bma250_set_int_data_sel(unsigned char intsel);
/* EasyCASE ) */
/* EasyCASE ( 1037
   bma250_get_int_data_sel */
int bma250_get_int_data_sel(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1039
   bma250_set_int2_pad_sel */
int bma250_set_int2_pad_sel(unsigned char int2sel);
/* EasyCASE ) */
/* EasyCASE ( 1040
   bma250_get_int2_pad_sel */
int bma250_get_int2_pad_sel(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1041
   bma250_set_int_src */
int bma250_set_int_src(unsigned char intsrc);
/* EasyCASE ) */
/* EasyCASE ( 1043
   bma250_get_int_src */
int bma250_get_int_src(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1044
   bma250_set_int_set */
int bma250_set_int_set(unsigned char intset,unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1045
   bma250_get_int_set */
int bma250_get_int_set(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1047
   bma250_get_mode_ctrl */
int bma250_get_mode_ctrl(unsigned char *mode);
/* EasyCASE ) */
/* EasyCASE ( 1048
   bma250_set_low_duration */
int bma250_set_low_g_duration(unsigned char duration);
/* EasyCASE ) */
/* EasyCASE ( 1049
   bma250_get_low_duration */
int bma250_get_low_g_duration(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1051
   bma250_set_low_g_threshold */
int bma250_set_low_g_threshold(unsigned char threshold);
/* EasyCASE ) */
/* EasyCASE ( 1052
   bma250_get_low_g_threshold */
int bma250_get_low_g_threshold(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1053
   bma250_set_high_g_duration */
int bma250_set_high_g_duration(unsigned char duration);
/* EasyCASE ) */
/* EasyCASE ( 1057
   bma250_get_high_g_duration */
int bma250_get_high_g_duration(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1058
   bma250_set_high_g_threshold */
int bma250_set_high_g_threshold(unsigned char threshold);
/* EasyCASE ) */
/* EasyCASE ( 1059
   bma250_get_high_g_threshold */
int bma250_get_high_g_threshold(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1060
   bma250_set_slope_duration */
int bma250_set_slope_duration(unsigned char duration);
/* EasyCASE ) */
/* EasyCASE ( 1062
   bma250_get_slope_duration */
int bma250_get_slope_duration(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1063
   bma250_set_slope_threshold */
int bma250_set_slope_threshold(unsigned char threshold);
/* EasyCASE ) */
/* EasyCASE ( 1064
   bma250_get_slope_threshold */
int bma250_get_slope_threshold(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1065
   bma250_set_tap_duration */
int bma250_set_tap_duration(unsigned char duration);
int bma250_get_tap_duration(unsigned char *status );

/* EasyCASE ) */
/* EasyCASE ( 1070
   bma250_set_tap_shock */
int bma250_set_tap_shock(unsigned char setval);
int bma250_get_tap_shock(unsigned char *status );

/* EasyCASE ) */
/* EasyCASE ( 1073
   bma250_get_tap_quiet */
int bma250_set_tap_quiet_duration(unsigned char duration);
int bma250_get_tap_quiet(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1075
   bma250_set_tap_threshold */
int bma250_set_tap_threshold(unsigned char threshold);
/* EasyCASE ) */
/* EasyCASE ( 1076
   bma250_get_tap_threshold */
int bma250_get_tap_threshold(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1078
   bma250_set_orient_mode */
int bma250_set_orient_mode(unsigned char mode);
/* EasyCASE ) */
/* EasyCASE ( 1079
   bma250_get_tap_samp */
int bma250_set_tap_samp(unsigned char samp);
int bma250_get_tap_samp(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1081
   bma250_get_orient_mode */
int bma250_get_orient_mode(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1082
   bma250_set_orient_blocking */
int bma250_set_orient_blocking(unsigned char samp);
/* EasyCASE ) */
/* EasyCASE ( 1086
   bma250_get_orient_blocking */
int bma250_get_orient_blocking(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1087
   bma250_set_orient_hyst */
int bma250_set_orient_hyst(unsigned char orienthyst);
/* EasyCASE ) */
/* EasyCASE ( 1089
   bma250_get_orient_hyst */
int bma250_get_orient_hyst(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1090
   bma250_set_theta_blocking */
int bma250_set_theta_blocking(unsigned char thetablk);
/* EasyCASE ) */
/* EasyCASE ( 1092
   bma250_get_theta_blocking */
int bma250_get_theta_blocking(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1093
   bma250_set_orient_ex */
int bma250_set_orient_ex(unsigned char orientex);
/* EasyCASE ) */
/* EasyCASE ( 1095
   bma250_get_orient_ex */
int bma250_get_orient_ex(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1096
   bma250_set_theta_flat */
int bma250_set_theta_flat(unsigned char thetaflat);
/* EasyCASE ) */
/* EasyCASE ( 1100
   bma250_get_theta_flat */
int bma250_get_theta_flat(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1102
   bma250_set_flat_holt_time */
int bma250_set_flat_hold_time(unsigned char holdtime);
/* EasyCASE ) */
/* EasyCASE ( 1104
   bma250_get_flat_holt_time */
int bma250_get_flat_hold_time(unsigned char *holdtime );
/* EasyCASE ) */
/* EasyCASE ( 1106
   bma250_get_low_power_state */
int bma250_get_low_power_state(unsigned char *Lowpower );
/* EasyCASE ) */
/* EasyCASE ( 1108
   bma250_set_selftest_st */
int bma250_set_selftest_st(unsigned char selftest);
/* EasyCASE ) */
/* EasyCASE ( 1112
   bma250_get_selftest_st */
int bma250_get_selftest_st(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1113
   bma250_set_selftest_stn */
int bma250_set_selftest_stn(unsigned char stn);
/* EasyCASE ) */
/* EasyCASE ( 1114
   bma250_get_selftest_stn */
int bma250_get_selftest_stn(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1115
   bma250_set_selftest_st_amp */
int bma250_set_selftest_st_amp(unsigned char stamp);
/* EasyCASE ) */
/* EasyCASE ( 1116
   bma250_get_selftest_st_amp */
int bma250_get_selftest_st_amp(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1121
   bma250_set_ee_w */
int bma250_set_ee_w(unsigned char eew);
/* EasyCASE ) */
/* EasyCASE ( 1117
   bma250_get_ee_w */
int bma250_get_ee_w(unsigned char *eew);
/* EasyCASE ) */
/* EasyCASE ( 1123
   bma250_set_ee_prog_trig */
int bma250_set_ee_prog_trig(unsigned char eeprog);
/* EasyCASE ) */
/* EasyCASE ( 1125
   bma250_get_eeprom_writing_status */
int bma250_get_eeprom_writing_status(unsigned char *eewrite );
/* EasyCASE ) */
/* EasyCASE ( 1127
   bma250_set_update_image */
int bma250_set_update_image(void);
/* EasyCASE ) */
/* EasyCASE ( 1129
   bma250_set_3wire_spi */
int bma250_set_3wire_spi(void);
/* EasyCASE ) */
/* EasyCASE ( 1133
   bma250_get_3wire_spi */
int bma250_get_3wire_spi(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1137
   bma250_set_i2c_wdt_timer */
int bma250_set_i2c_wdt_timer(unsigned char timedly);
/* EasyCASE ) */
/* EasyCASE ( 1139
   bma250_get_i2c_wdt_timer */
int bma250_get_i2c_wdt_timer(unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1141
   bma250_set_unlock_trimming_part */
int bma250_set_unlock_trimming_part(void);
/* EasyCASE ) */
/* EasyCASE ( 1143
   bma250_set_hp_en */
int bma250_set_hp_en(unsigned char param,unsigned char hpval);
/* EasyCASE ) */
/* EasyCASE ( 1145
   bma250_get_hp_en */
int bma250_get_hp_en(unsigned char param,unsigned char *status );
/* EasyCASE ) */
/* EasyCASE ( 1149
   bma250_get_cal_ready */
int bma250_get_cal_ready(unsigned char *calrdy );
/* EasyCASE ) */
/* EasyCASE ( 1155
   bma250_set_cal_trigger */
int bma250_set_cal_trigger(unsigned char caltrigger);
/* EasyCASE ) */
/* EasyCASE ( 1159
   bma250_set_offset_reset */
int bma250_set_offset_reset(void);
/* EasyCASE ) */
/* EasyCASE ( 1161
   bma250_set_offset_cutoff */
int bma250_set_offset_cutoff(unsigned char offsetcutoff);
/* EasyCASE ) */
/* EasyCASE ( 1151
   bma250_get_offset_cutoff */
int bma250_get_offset_cutoff(unsigned char *cutoff );
/* EasyCASE ) */
/* EasyCASE ( 1165
   bma250_set_offset_target_x */
int bma250_set_offset_target_x(unsigned char offsettarget);
/* EasyCASE ) */
/* EasyCASE ( 1175
   bma250_get_offset_target_x */
int bma250_get_offset_target_x(unsigned char *offsettarget );
/* EasyCASE ) */
/* EasyCASE ( 1179
   bma250_set_offset_target_y */
int bma250_set_offset_target_y(unsigned char offsettarget);
/* EasyCASE ) */
/* EasyCASE ( 1183
   bma250_get_offset_target_y */
int bma250_get_offset_target_y(unsigned char *offsettarget );
/* EasyCASE ) */
/* EasyCASE ( 1185
   bma250_set_offset_target_z */
int bma250_set_offset_target_z(unsigned char offsettarget);
/* EasyCASE ) */
/* EasyCASE ( 1187
   bma250_get_offset_target_z */
int bma250_get_offset_target_z(unsigned char *offsettarget );
/* EasyCASE ) */
/* EasyCASE ( 1189
   bma250_set_offset_filt_x */
int bma250_set_offset_filt_x(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1191
   bma250_get_offset_filt_x */
int bma250_get_offset_filt_x(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1195
   bma250_set_offset_filt_y */
int bma250_set_offset_filt_y(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1197
   bma250_get_offset_filt_y */
int bma250_get_offset_filt_y(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1199
   bma250_set_offset_filt_z */
int bma250_set_offset_filt_z(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1201
   bma250_get_offset_filt_z */
int bma250_get_offset_filt_z(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1203
   bma250_set_offset_unfilt_x */
int bma250_set_offset_unfilt_x(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1207
   bma250_get_offset_unfilt_x */
int bma250_get_offset_unfilt_x(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1209
   bma250_set_offset_unfilt_y */
int bma250_set_offset_unfilt_y(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1177
   bma250_get_offset_unfilt_y */
int bma250_get_offset_unfilt_y(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1169
   bma250_set_offset_unfilt_z */
int bma250_set_offset_unfilt_z(unsigned char offsetfilt);
/* EasyCASE ) */
/* EasyCASE ( 1213
   bma250_get_offset_unfilt_z */
int bma250_get_offset_unfilt_z(unsigned char *offsetfilt );
/* EasyCASE ) */
/* EasyCASE ( 1215
   bma250_set_Int_Mode */
int bma250_set_Int_Mode(unsigned char Mode );
/* EasyCASE ) */
/* EasyCASE ( 1217
   bma250_get_Int_Mode */
int bma250_get_Int_Mode(unsigned char * Mode );
/* EasyCASE ) */
/* EasyCASE ( 1219
   bma250_set_Int_Enable */
int bma250_set_Int_Enable(unsigned char InterruptType , unsigned char value );
/* EasyCASE ) */
/* EasyCASE ( 1221
   bma250_write_ee */
int bma250_write_ee(unsigned char addr, unsigned char data);
/* EasyCASE ) */
/* EasyCASE ( 1171
   bma250_set_low_hy */
int bma250_set_low_hy(unsigned char hysval);
/* EasyCASE ) */
/* EasyCASE ( 1225
   bma250_set_high_hy */
int bma250_set_high_hy(unsigned char hysval);
/* EasyCASE ) */
/* EasyCASE ( 1227
   bma250_set_low_mode */
int bma250_set_low_mode(unsigned char state);
/* EasyCASE ) */
/* EasyCASE ( 1229
   bma250_get_update_image_status */
int bma250_get_update_image_status(unsigned char *imagestatus );
/* EasyCASE ) */
/* EasyCASE ) */

#endif
/* EasyCASE ) */
