/*  $Date: 2008/06/03 16:22:36 $
 *  $Revision: 1.8 $
 *
 */


/** \mainpage SMB380 Acceleration Sensor API
 * Copyright (C) 2007 Bosch Sensortec GmbH
 *  \section intro_sec Introduction
 * SMB380 3-axis digital Accelerometer Programming Interface
 * The SMB380 API enables quick access to Bosch Sensortec's 3-Axis digital accelerometer.
 * The only mandatory steps are: 
 *
 * 1. linking the target application's communication functions to the API (\ref SMB380_WR_FUNC_PTR, \ref SMB380_RD_FUNC_PTR)
 *
 * 2. calling the smb380_init() routine, which initializes all necessary data structures for using all functions
 *
 * Author:	Lars.Beseke@bosch-sensortec.com
 *
 * 
 * \section disclaimer_sec Disclaimer
 *
 *
 *
 * Common:
 * Bosch Sensortec products are developed for the consumer goods industry. They may only be used 
 * within the parameters of the respective valid product data sheet.  Bosch Sensortec products are 
 * provided with the express understanding that there is no warranty of fitness for a particular purpose. 
 * They are not fit for use in life-sustaining, safety or security sensitive systems or any system or device 
 * that may lead to bodily harm or property damage if the system or device malfunctions. In addition, 
 * Bosch Sensortec products are not fit for use in products which interact with motor vehicle systems.  
 * The resale and/or use of products are at the purchaser?s own risk and his own responsibility. The 
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

 /** \file smb380.h
    \brief Header file for all #define constants and function prototypes
  
    
*/



#ifndef __SMB380_H__
#define __SMB380_H__



/* SMB380 Macro for read and write commincation */


/**
   define for used read and write macros 
*/


/** Define the calling convention of YOUR bus communication routine.
	\note This includes types of parameters. This example shows the configuration for an SPI bus link.

	If your communication function looks like this:

	write_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

	The SMB380_WR_FUNC_PTR would equal:

    #define	SMB380_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char, unsigned char *, unsigned char)
	
	Parameters can be mixed as needed refer to the \ref SMB380_BUS_WRITE_FUNC  macro.
	

*/
#define SMB380_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char *, unsigned char)



/** link makro between API function calls and bus write function
	\note The bus write function can change since this is a system dependant issue.

	If the bus_write parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

	If the parameters are differently ordered or your communication function like I2C need to know the device address, 
	you can change this macro accordingly.


	define SMB380_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_write(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the \ref SMB380_WR_FUNC_PTR definition.


	      
*/
#define SMB380_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_write(reg_addr, reg_data, wr_len)


/** Define the calling convention of YOUR bus communication routine.
	\note This includes types of parameters. This example shows the configuration for an SPI bus link.

	If your communication function looks like this:

	read_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

	The SMB380_RD_FUNC_PTR would equal:

    #define	SMB380_RD_FUNC_PTR char (* bus_read)(unsigned char, unsigned char, unsigned char *, unsigned char)
	
	Parameters can be mixed as needed refer to the \ref SMB380_BUS_READ_FUNC  macro.
	

*/

#define SMB380_SPI_RD_MASK 0x80   /* for spi read transactions on SPI the MSB has to be set */
#define SMB380_RD_FUNC_PTR char (* bus_read)( unsigned char, unsigned char *, unsigned char)


/** link makro between API function calls and bus read function
	\note The bus write function can change since this is a system dependant issue.

	If the bus_read parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

	If the parameters are differently ordered or your communication function like I2C need to know the device address, 
	you can change this macro accordingly.


	define SMB380_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_read(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the \ref SMB380_WR_FUNC_PTR definition.

	\note: this macro also includes the "MSB='1'" for reading SMB380 addresses. 
      
*/
#define SMB380_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
           bus_read(reg_addr | SMB380_SPI_RD_MASK, reg_data, r_len)







/** SMB380 I2C Address
*/

#define SMB380_I2C_ADDR		0x38


/*
	SMB380 API error codes
*/

#define E_SMB_NULL_PTR		(char)-127
#define E_COMM_RES		    (char)-1
#define E_OUT_OF_RANGE		(char)-2


/* 
 *	
 *	register definitions 	
 *
 */


#define SMB380_EEP_OFFSET   0x20
#define SMB380_IMAGE_BASE	0x0b
#define SMB380_IMAGE_LEN	19

#define SMB380_CHIP_ID_REG			0x00
#define SMB380_VERSION_REG			0x01
#define SMB380_X_AXIS_LSB_REG		0x02
#define SMB380_X_AXIS_MSB_REG		0x03
#define SMB380_Y_AXIS_LSB_REG		0x04
#define SMB380_Y_AXIS_MSB_REG		0x05
#define SMB380_Z_AXIS_LSB_REG		0x06
#define SMB380_Z_AXIS_MSB_REG		0x07
#define SMB380_TEMP_RD_REG			0x08
#define SMB380_STATUS_REG	0x09
#define SMB380_CTRL_REG		0x0a
#define SMB380_CONF1_REG	0x0b
#define SMB380_LG_THRESHOLD_REG	0x0c
#define SMB380_LG_DURATION_REG		0x0d
#define SMB380_HG_THRESHOLD_REG	0x0e
#define SMB380_HG_DURATION_REG		0x0f
#define SMB380_MOTION_THRS_REG		0x10
#define SMB380_HYSTERESIS_REG		0x11
#define SMB380_CUSTOMER1_REG		0x12
#define SMB380_CUSTOMER2_REG		0x13
#define SMB380_RANGE_BWIDTH_REG	0x14
#define SMB380_CONF2_REG	0x15

#define SMB380_OFFS_GAIN_X_REG		0x16
#define SMB380_OFFS_GAIN_Y_REG		0x17
#define SMB380_OFFS_GAIN_Z_REG		0x18
#define SMB380_OFFS_GAIN_T_REG		0x19
#define SMB380_OFFSET_X_REG		0x1a
#define SMB380_OFFSET_Y_REG		0x1b
#define SMB380_OFFSET_Z_REG		0x1c
#define SMB380_OFFSET_T_REG		0x1d


/* register write and read delays */

#define SMB380_MDELAY_DATA_TYPE	unsigned int
#define SMB380_EE_W_DELAY 28	/* delay after EEP write is 28 msec */



/** SMB380 acceleration data 
	\brief Structure containing acceleration values for x,y and z-axis in signed short

*/

typedef struct  {
		short x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
			  y, /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
			  z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
} smb380acc_t;


/** SMB380 image registers data structure
	\brief Register type that contains all SMB380 image registers from address 0x0b to 0x15
	This structure can hold the complete image data of SMB380

*/
typedef struct  {
		unsigned char	
		smb380_conf1 ,  /**<  image address 0x0b: interrupt enable bits, low-g settings */
		lg_threshold,	/**<  image address 0x0c: low-g threshold, depends on selected g-range */
		lg_duration,	/**<  image address 0x0d: low-g duration in ms */
		hg_threshold,	/**<  image address 0x0e: high-g threshold, depends on selected g-range */
		hg_duration,	/**<  image address 0x0f: high-g duration in ms */
		motion_thrs,	/**<  image address 0x10: any motion threshold */
		hysteresis,		/**<  image address 0x11: low-g and high-g hysteresis register */
		customer1,		/**<  image address 0x12: customer reserved register 1 */
		customer2,		/**<  image address 0x13: customer reserved register 2  */
		range_bwidth,	/**<  image address 0x14: range and bandwidth selection register */
		smb380_conf2,	/**<  image address 0x15: spi4, latched interrupt, auto-wake-up configuration */
		offs_gain_x,	/**<  image address 0x16: offset_x LSB and x-axis gain settings */
		offs_gain_y,	/**<  image address 0x17: offset_y LSB and y-axis gain settings */
		offs_gain_z,	/**<  image address 0x18: offset_z LSB and z-axis gain settings */
		offs_gain_t,	/**<  image address 0x19: offset_t LSB and temperature gain settings */
		offset_x,		/**<  image address 0x1a: offset_x calibration MSB register */
		offset_y,		/**<  image address 0x1b: offset_y calibration MSB register */ 
		offset_z,		/**<  image address 0x1c: offset_z calibration MSB register */ 
		offset_t;		/**<  image address 0x1d: temperature calibration MSB register */ 
} smb380regs_t;


/** smb380 typedef structure
	\brief This structure holds all relevant information about SMB380 and links communication to the 
*/

typedef struct {	
	smb380regs_t * image;	/**< pointer to smb380regs_t structure not mandatory */
	unsigned char mode;		/**< save current SMB380 operation mode */
	unsigned char chip_id,	/**< save SMB380's chip id which has to be 0x02 after calling smb380_init() */
				  ml_version, /**< holds the SMB380 ML_version number */	
				  al_version; /**< holds the SMB380 AL_version number */
	unsigned char dev_addr;   /**< initializes SMB380's I2C device address 0x38 */
	unsigned char int_mask;	  /**< stores the current SMB380 API generated interrupt mask */
	SMB380_WR_FUNC_PTR;		  /**< function pointer to the SPI/I2C write function */
	SMB380_RD_FUNC_PTR;		  /**< function pointer to the SPI/I2C read function */
	void (*delay_msec)( SMB380_MDELAY_DATA_TYPE ); /**< function pointer to a pause in mili seconds function */
} smb380_t;




	
/* 
 *	
 *	bit slice positions in registers
 *
 */

/** \cond BITSLICE */

#define SMB380_CHIP_ID__POS		0
#define SMB380_CHIP_ID__MSK		0x07
#define SMB380_CHIP_ID__LEN		3
#define SMB380_CHIP_ID__REG		SMB380_CHIP_ID_REG


#define SMB380_ML_VERSION__POS		0
#define SMB380_ML_VERSION__LEN		4
#define SMB380_ML_VERSION__MSK		0x0F
#define SMB380_ML_VERSION__REG		SMB380_VERSION_REG



#define SMB380_AL_VERSION__POS  	4
#define SMB380_AL_VERSION__LEN  	4
#define SMB380_AL_VERSION__MSK		0xF0
#define SMB380_AL_VERSION__REG		SMB380_VERSION_REG


/* DATA REGISTERS */


#define SMB380_NEW_DATA_X__POS  	0
#define SMB380_NEW_DATA_X__LEN  	1
#define SMB380_NEW_DATA_X__MSK  	0x01
#define SMB380_NEW_DATA_X__REG		SMB380_X_AXIS_LSB_REG

#define SMB380_ACC_X_LSB__POS   	6
#define SMB380_ACC_X_LSB__LEN   	2
#define SMB380_ACC_X_LSB__MSK		0xC0
#define SMB380_ACC_X_LSB__REG		SMB380_X_AXIS_LSB_REG

#define SMB380_ACC_X_MSB__POS   	0
#define SMB380_ACC_X_MSB__LEN   	8
#define SMB380_ACC_X_MSB__MSK		0xFF
#define SMB380_ACC_X_MSB__REG		SMB380_X_AXIS_MSB_REG

#define SMB380_NEW_DATA_Y__POS  	0
#define SMB380_NEW_DATA_Y__LEN  	1
#define SMB380_NEW_DATA_Y__MSK  	0x01
#define SMB380_NEW_DATA_Y__REG		SMB380_Y_AXIS_LSB_REG

#define SMB380_ACC_Y_LSB__POS   	6
#define SMB380_ACC_Y_LSB__LEN   	2
#define SMB380_ACC_Y_LSB__MSK   	0xC0
#define SMB380_ACC_Y_LSB__REG		SMB380_Y_AXIS_LSB_REG

#define SMB380_ACC_Y_MSB__POS   	0
#define SMB380_ACC_Y_MSB__LEN   	8
#define SMB380_ACC_Y_MSB__MSK   	0xFF
#define SMB380_ACC_Y_MSB__REG		SMB380_Y_AXIS_MSB_REG

#define SMB380_NEW_DATA_Z__POS  	0
#define SMB380_NEW_DATA_Z__LEN  	1
#define SMB380_NEW_DATA_Z__MSK		0x01
#define SMB380_NEW_DATA_Z__REG		SMB380_Z_AXIS_LSB_REG

#define SMB380_ACC_Z_LSB__POS   	6
#define SMB380_ACC_Z_LSB__LEN   	2
#define SMB380_ACC_Z_LSB__MSK		0xC0
#define SMB380_ACC_Z_LSB__REG		SMB380_Z_AXIS_LSB_REG

#define SMB380_ACC_Z_MSB__POS   	0
#define SMB380_ACC_Z_MSB__LEN   	8
#define SMB380_ACC_Z_MSB__MSK		0xFF
#define SMB380_ACC_Z_MSB__REG		SMB380_Z_AXIS_MSB_REG

#define SMB380_TEMPERATURE__POS 	0
#define SMB380_TEMPERATURE__LEN 	8
#define SMB380_TEMPERATURE__MSK 	0xFF
#define SMB380_TEMPERATURE__REG		SMB380_TEMP_RD_REG




/* STATUS BITS */

#define SMB380_STATUS_HG__POS		0
#define SMB380_STATUS_HG__LEN		1
#define SMB380_STATUS_HG__MSK		0x01
#define SMB380_STATUS_HG__REG		SMB380_STATUS_REG

#define SMB380_STATUS_LG__POS		1
#define SMB380_STATUS_LG__LEN		1
#define SMB380_STATUS_LG__MSK		0x02
#define SMB380_STATUS_LG__REG		SMB380_STATUS_REG

#define SMB380_HG_LATCHED__POS  	2
#define SMB380_HG_LATCHED__LEN  	1
#define SMB380_HG_LATCHED__MSK		0x04
#define SMB380_HG_LATCHED__REG		SMB380_STATUS_REG

#define SMB380_LG_LATCHED__POS		3
#define SMB380_LG_LATCHED__LEN		1
#define SMB380_LG_LATCHED__MSK		8
#define SMB380_LG_LATCHED__REG		SMB380_STATUS_REG

#define SMB380_ALERT_PHASE__POS		4
#define SMB380_ALERT_PHASE__LEN		1
#define SMB380_ALERT_PHASE__MSK		0x10
#define SMB380_ALERT_PHASE__REG		SMB380_STATUS_REG


#define SMB380_ST_RESULT__POS		7
#define SMB380_ST_RESULT__LEN		1
#define SMB380_ST_RESULT__MSK		0x80
#define SMB380_ST_RESULT__REG		SMB380_STATUS_REG


/* CONTROL BITS */

#define SMB380_SLEEP__POS			0
#define SMB380_SLEEP__LEN			1
#define SMB380_SLEEP__MSK			0x01
#define SMB380_SLEEP__REG			SMB380_CTRL_REG

#define SMB380_SOFT_RESET__POS		1
#define SMB380_SOFT_RESET__LEN		1
#define SMB380_SOFT_RESET__MSK		0x02
#define SMB380_SOFT_RESET__REG		SMB380_CTRL_REG





#define SMB380_SELF_TEST__POS		2
#define SMB380_SELF_TEST__LEN		2
#define SMB380_SELF_TEST__MSK		0x0C
#define SMB380_SELF_TEST__REG		SMB380_CTRL_REG




#define SMB380_SELF_TEST0__POS		2
#define SMB380_SELF_TEST0__LEN		1
#define SMB380_SELF_TEST0__MSK		0x04
#define SMB380_SELF_TEST0__REG		SMB380_CTRL_REG

#define SMB380_SELF_TEST1__POS		3
#define SMB380_SELF_TEST1__LEN		1
#define SMB380_SELF_TEST1__MSK		0x08
#define SMB380_SELF_TEST1__REG		SMB380_CTRL_REG




#define SMB380_EE_W__POS			4
#define SMB380_EE_W__LEN			1
#define SMB380_EE_W__MSK			0x10
#define SMB380_EE_W__REG			SMB380_CTRL_REG

#define SMB380_UPDATE_IMAGE__POS	5
#define SMB380_UPDATE_IMAGE__LEN	1
#define SMB380_UPDATE_IMAGE__MSK	0x20
#define SMB380_UPDATE_IMAGE__REG	SMB380_CTRL_REG

#define SMB380_RESET_INT__POS		6
#define SMB380_RESET_INT__LEN		1
#define SMB380_RESET_INT__MSK		0x40
#define SMB380_RESET_INT__REG		SMB380_CTRL_REG



/* LOW-G, HIGH-G settings */



#define SMB380_ENABLE_LG__POS		0
#define SMB380_ENABLE_LG__LEN		1
#define SMB380_ENABLE_LG__MSK		0x01
#define SMB380_ENABLE_LG__REG		SMB380_CONF1_REG




#define SMB380_ENABLE_HG__POS		1
#define SMB380_ENABLE_HG__LEN		1
#define SMB380_ENABLE_HG__MSK		0x02
#define SMB380_ENABLE_HG__REG		SMB380_CONF1_REG


/* LG/HG counter */

	

#define SMB380_COUNTER_LG__POS			2
#define SMB380_COUNTER_LG__LEN			2
#define SMB380_COUNTER_LG__MSK			0x0C
#define SMB380_COUNTER_LG__REG			SMB380_CONF1_REG
	
#define SMB380_COUNTER_HG__POS			4
#define SMB380_COUNTER_HG__LEN			2
#define SMB380_COUNTER_HG__MSK			0x30
#define SMB380_COUNTER_HG__REG			SMB380_CONF1_REG




/* LG/HG duration is in ms */

#define SMB380_LG_DUR__POS			0
#define SMB380_LG_DUR__LEN			8
#define SMB380_LG_DUR__MSK			0xFF
#define SMB380_LG_DUR__REG			SMB380_LG_DURATION_REG

#define SMB380_HG_DUR__POS			0
#define SMB380_HG_DUR__LEN			8
#define SMB380_HG_DUR__MSK			0xFF
#define SMB380_HG_DUR__REG			SMB380_HG_DURATION_REG




				

#define SMB380_LG_THRES__POS		0
#define SMB380_LG_THRES__LEN		8
#define SMB380_LG_THRES__MSK		0xFF
#define SMB380_LG_THRES__REG		SMB380_LG_THRESHOLD_REG





#define SMB380_HG_THRES__POS		0
#define SMB380_HG_THRES__LEN		8
#define SMB380_HG_THRES__MSK		0xFF
#define SMB380_HG_THRES__REG		SMB380_HG_THRESHOLD_REG








#define SMB380_LG_HYST__POS			0
#define SMB380_LG_HYST__LEN			3
#define SMB380_LG_HYST__MSK			0x07
#define SMB380_LG_HYST__REG			SMB380_HYSTERESIS_REG




#define SMB380_HG_HYST__POS			3
#define SMB380_HG_HYST__LEN			3
#define SMB380_HG_HYST__MSK			0x38
#define SMB380_HG_HYST__REG			SMB380_HYSTERESIS_REG


/* ANY MOTION and ALERT settings */

#define SMB380_EN_ANY_MOTION__POS		6
#define SMB380_EN_ANY_MOTION__LEN		1
#define SMB380_EN_ANY_MOTION__MSK		0x40
#define SMB380_EN_ANY_MOTION__REG		SMB380_CONF1_REG


/* ALERT settings */


#define SMB380_ALERT__POS			7
#define SMB380_ALERT__LEN			1
#define SMB380_ALERT__MSK			0x80
#define SMB380_ALERT__REG			SMB380_CONF1_REG


/* ANY MOTION Duration */




#define SMB380_ANY_MOTION_THRES__POS	0
#define SMB380_ANY_MOTION_THRES__LEN	8
#define SMB380_ANY_MOTION_THRES__MSK	0xFF
#define SMB380_ANY_MOTION_THRES__REG	SMB380_MOTION_THRS_REG




#define SMB380_ANY_MOTION_DUR__POS		6
#define SMB380_ANY_MOTION_DUR__LEN		2
#define SMB380_ANY_MOTION_DUR__MSK		0xC0	
#define SMB380_ANY_MOTION_DUR__REG		SMB380_HYSTERESIS_REG


#define SMB380_CUSTOMER_RESERVED1__POS		0
#define SMB380_CUSTOMER_RESERVED1__LEN	 	8
#define SMB380_CUSTOMER_RESERVED1__MSK		0xFF
#define SMB380_CUSTOMER_RESERVED1__REG		SMB380_CUSTOMER1_REG

#define SMB380_CUSTOMER_RESERVED2__POS		0
#define SMB380_CUSTOMER_RESERVED2__LEN	 	8
#define SMB380_CUSTOMER_RESERVED2__MSK		0xFF
#define SMB380_CUSTOMER_RESERVED2__REG		SMB380_CUSTOMER2_REG



/* BANDWIDTH dependend definitions */

#define SMB380_BANDWIDTH__POS				0
#define SMB380_BANDWIDTH__LEN			 	3
#define SMB380_BANDWIDTH__MSK			 	0x07
#define SMB380_BANDWIDTH__REG				SMB380_RANGE_BWIDTH_REG




/* RANGE */

#define SMB380_RANGE__POS				3
#define SMB380_RANGE__LEN				2
#define SMB380_RANGE__MSK				0x18	
#define SMB380_RANGE__REG				SMB380_RANGE_BWIDTH_REG


/* WAKE UP */



#define SMB380_WAKE_UP__POS			0
#define SMB380_WAKE_UP__LEN			1
#define SMB380_WAKE_UP__MSK			0x01
#define SMB380_WAKE_UP__REG			SMB380_CONF2_REG




#define SMB380_WAKE_UP_PAUSE__POS		1
#define SMB380_WAKE_UP_PAUSE__LEN		2
#define SMB380_WAKE_UP_PAUSE__MSK		0x06
#define SMB380_WAKE_UP_PAUSE__REG		SMB380_CONF2_REG


/* ACCELERATION DATA SHADOW */



#define SMB380_SHADOW_DIS__POS			3
#define SMB380_SHADOW_DIS__LEN			1
#define SMB380_SHADOW_DIS__MSK			0x08
#define SMB380_SHADOW_DIS__REG			SMB380_CONF2_REG


/* LATCH Interrupt */



#define SMB380_LATCH_INT__POS			4
#define SMB380_LATCH_INT__LEN			1
#define SMB380_LATCH_INT__MSK			0x10
#define SMB380_LATCH_INT__REG			SMB380_CONF2_REG

/* new data interrupt */


#define SMB380_NEW_DATA_INT__POS		5
#define SMB380_NEW_DATA_INT__LEN		1
#define SMB380_NEW_DATA_INT__MSK		0x20
#define SMB380_NEW_DATA_INT__REG		SMB380_CONF2_REG



#define SMB380_ENABLE_ADV_INT__POS		6
#define SMB380_ENABLE_ADV_INT__LEN		1
#define SMB380_ENABLE_ADV_INT__MSK		0x40
#define SMB380_ENABLE_ADV_INT__REG		SMB380_CONF2_REG


#define SMB380_SMB380_SPI4_OFF	0
#define SMB380_SMB380_SPI4_ON	1

#define SMB380_SPI4__POS				7
#define SMB380_SPI4__LEN				1
#define SMB380_SPI4__MSK				0x80
#define SMB380_SPI4__REG				SMB380_CONF2_REG


#define SMB380_OFFSET_X_LSB__POS	6
#define SMB380_OFFSET_X_LSB__LEN	2
#define SMB380_OFFSET_X_LSB__MSK	0xC0
#define SMB380_OFFSET_X_LSB__REG	SMB380_OFFS_GAIN_X_REG

#define SMB380_GAIN_X__POS			0
#define SMB380_GAIN_X__LEN			6
#define SMB380_GAIN_X__MSK			0x3f
#define SMB380_GAIN_X__REG			SMB380_OFFS_GAIN_X_REG


#define SMB380_OFFSET_Y_LSB__POS	6
#define SMB380_OFFSET_Y_LSB__LEN	2
#define SMB380_OFFSET_Y_LSB__MSK	0xC0
#define SMB380_OFFSET_Y_LSB__REG	SMB380_OFFS_GAIN_Y_REG

#define SMB380_GAIN_Y__POS			0
#define SMB380_GAIN_Y__LEN			6
#define SMB380_GAIN_Y__MSK			0x3f
#define SMB380_GAIN_Y__REG			SMB380_OFFS_GAIN_Y_REG


#define SMB380_OFFSET_Z_LSB__POS	6
#define SMB380_OFFSET_Z_LSB__LEN	2
#define SMB380_OFFSET_Z_LSB__MSK	0xC0
#define SMB380_OFFSET_Z_LSB__REG	SMB380_OFFS_GAIN_Z_REG

#define SMB380_GAIN_Z__POS			0
#define SMB380_GAIN_Z__LEN			6
#define SMB380_GAIN_Z__MSK			0x3f
#define SMB380_GAIN_Z__REG			SMB380_OFFS_GAIN_Z_REG

#define SMB380_OFFSET_T_LSB__POS	6
#define SMB380_OFFSET_T_LSB__LEN	2
#define SMB380_OFFSET_T_LSB__MSK	0xC0
#define SMB380_OFFSET_T_LSB__REG	SMB380_OFFS_GAIN_T_REG

#define SMB380_GAIN_T__POS			0
#define SMB380_GAIN_T__LEN			6
#define SMB380_GAIN_T__MSK			0x3f
#define SMB380_GAIN_T__REG			SMB380_OFFS_GAIN_T_REG

#define SMB380_OFFSET_X_MSB__POS	0
#define SMB380_OFFSET_X_MSB__LEN	8
#define SMB380_OFFSET_X_MSB__MSK	0xFF
#define SMB380_OFFSET_X_MSB__REG	SMB380_OFFSET_X_REG


#define SMB380_OFFSET_Y_MSB__POS	0
#define SMB380_OFFSET_Y_MSB__LEN	8
#define SMB380_OFFSET_Y_MSB__MSK	0xFF
#define SMB380_OFFSET_Y_MSB__REG	SMB380_OFFSET_Y_REG

#define SMB380_OFFSET_Z_MSB__POS	0
#define SMB380_OFFSET_Z_MSB__LEN	8
#define SMB380_OFFSET_Z_MSB__MSK	0xFF
#define SMB380_OFFSET_Z_MSB__REG	SMB380_OFFSET_Z_REG

#define SMB380_OFFSET_T_MSB__POS	0
#define SMB380_OFFSET_T_MSB__LEN	8
#define SMB380_OFFSET_T_MSB__MSK	0xFF
#define SMB380_OFFSET_T_MSB__REG	SMB380_OFFSET_T_REG





#define SMB380_GET_BITSLICE(regvar, bitname)\
			(regvar & bitname##__MSK) >> bitname##__POS


#define SMB380_SET_BITSLICE(regvar, bitname, val)\
		  (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  


/** \endcond */


/* CONSTANTS */


/* range and bandwidth */

#define SMB380_RANGE_2G			0 /**< sets range to 2G mode \see smb380_set_range() */
#define SMB380_RANGE_4G			1 /**< sets range to 4G mode \see smb380_set_range() */
#define SMB380_RANGE_8G			2 /**< sets range to 8G mode \see smb380_set_range() */


#define SMB380_BW_25HZ		0	/**< sets bandwidth to 25HZ \see smb380_set_bandwidth() */
#define SMB380_BW_50HZ		1	/**< sets bandwidth to 50HZ \see smb380_set_bandwidth() */
#define SMB380_BW_100HZ		2	/**< sets bandwidth to 100HZ \see smb380_set_bandwidth() */
#define SMB380_BW_190HZ		3	/**< sets bandwidth to 190HZ \see smb380_set_bandwidth() */
#define SMB380_BW_375HZ		4	/**< sets bandwidth to 375HZ \see smb380_set_bandwidth() */
#define SMB380_BW_750HZ		5	/**< sets bandwidth to 750HZ \see smb380_set_bandwidth() */
#define SMB380_BW_1500HZ	6	/**< sets bandwidth to 1500HZ \see smb380_set_bandwidth() */

/* mode settings */

#define SMB380_MODE_NORMAL      0
#define SMB380_MODE_SLEEP       2
#define SMB380_MODE_WAKE_UP     3

/* wake up */

#define SMB380_WAKE_UP_PAUSE_20MS		0
#define SMB380_WAKE_UP_PAUSE_80MS		1
#define SMB380_WAKE_UP_PAUSE_320MS		2
#define SMB380_WAKE_UP_PAUSE_2560MS		3


/* LG/HG thresholds are in LSB and depend on RANGE setting */
/* no range check on threshold calculation */

#define SMB380_SELF_TEST0_ON		1
#define SMB380_SELF_TEST1_ON		2

#define SMB380_EE_W_OFF			0
#define SMB380_EE_W_ON			1



/* low-g, high-g, any_motion */


#define SMB380_COUNTER_LG_RST		0
#define SMB380_COUNTER_LG_0LSB		SMB380_COUNTER_LG_RST
#define SMB380_COUNTER_LG_1LSB		1
#define SMB380_COUNTER_LG_2LSB		2
#define SMB380_COUNTER_LG_3LSB		3

#define SMB380_COUNTER_HG_RST		0
#define SMB380_COUNTER_HG_0LSB		SMB380_COUNTER_HG_RST
#define SMB380_COUNTER_HG_1LSB		1
#define SMB380_COUNTER_HG_2LSB		2
#define SMB380_COUNTER_HG_3LSB		3

#define SMB380_COUNTER_RST			0
#define SMB380_COUNTER_0LSB			SMB380_COUNTER_RST
#define SMB380_COUNTER_1LSB			1
#define SMB380_COUNTER_2LSB			2
#define SMB380_COUNTER_3LSB			3



/** Macro to convert floating point low-g-thresholds in G to 8-bit register values.<br>
  * Example: SMB380_LG_THRES_IN_G( 0.3, 2.0) generates the register value for 0.3G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define SMB380_LG_THRES_IN_G( gthres, range)			((256 * gthres ) / range)

/** Macro to convert floating point high-g-thresholds in G to 8-bit register values.<br>
  * Example: SMB380_HG_THRES_IN_G( 1.4, 2.0) generates the register value for 1.4G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define SMB380_HG_THRES_IN_G(gthres, range)				((256 * gthres ) / range)

/** Macro to convert floating point low-g-hysteresis in G to 8-bit register values.<br>
  * Example: SMB380_LG_HYST_THRES_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define SMB380_LG_HYST_IN_G( ghyst, range )				((32 * ghyst) / range)

/** Macro to convert floating point high-g-hysteresis in G to 8-bit register values.<br>
  * Example: SMB380_HG_HYST_THRES_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */
#define SMB380_HG_HYST_IN_G( ghyst, range )				((32 * ghyst) / range)


/** Macro to convert floating point G-thresholds to 8-bit register values<br>
  * Example: SMB380_ANY_MOTION_THRES_IN_G( 1.2, 2.0) generates the register value for 1.2G threshold in 2G mode.
  * \brief convert g-values to 8-bit value
 */

#define SMB380_ANY_MOTION_THRES_IN_G( gthres, range)	((128 * gthres ) / range)


#define SMB380_ANY_MOTION_DUR_1		0
#define SMB380_ANY_MOTION_DUR_3		1
#define SMB380_ANY_MOTION_DUR_5		2
#define SMB380_ANY_MOTION_DUR_7		3



#define SMB380_SHADOW_DIS_OFF	0
#define SMB380_SHADOW_DIS_ON	1

#define SMB380_LATCH_INT_OFF	0
#define SMB380_LATCH_INT_ON		1

#define SMB380_NEW_DATA_INT_OFF	0
#define SMB380_NEW_DATA_INT_ON	1

#define SMB380_ENABLE_ADV_INT_OFF	0
#define SMB380_ENABLE_ADV_INT_ON	1

#define SMB380_EN_ANY_MOTION_OFF 	0
#define SMB380_EN_ANY_MOTION_ON 	1


#define SMB380_ALERT_OFF	0
#define SMB380_ALERT_ON		1

#define SMB380_ENABLE_LG_OFF	0
#define SMB380_ENABLE_LG_ON		1

#define SMB380_ENABLE_HG_OFF	0
#define SMB380_ENABLE_HG_ON		1



#define SMB380_INT_ALERT		(1<<7)
#define SMB380_INT_ANY_MOTION	(1<<6)
#define SMB380_INT_EN_ADV_INT	(1<<5)
#define SMB380_INT_NEW_DATA		(1<<4)
#define SMB380_INT_LATCH		(1<<3)
#define SMB380_INT_HG			(1<<1)
#define SMB380_INT_LG			(1<<0)


#define SMB380_INT_STATUS_HG			(1<<0)
#define SMB380_INT_STATUS_LG			(1<<1)
#define SMB380_INT_STATUS_HG_LATCHED	(1<<2)
#define SMB380_INT_STATUS_LG_LATCHED	(1<<3)
#define SMB380_INT_STATUS_ALERT			(1<<4)
#define SMB380_INT_STATUS_ST_RESULT		(1<<7)


#define SMB380_CONF1_INT_MSK	((1<<SMB380_ALERT__POS) | (1<<SMB380_EN_ANY_MOTION__POS) | (1<<SMB380_ENABLE_HG__POS) | (1<<SMB380_ENABLE_LG__POS))
#define SMB380_CONF2_INT_MSK	((1<<SMB380_ENABLE_ADV_INT__POS) | (1<<SMB380_NEW_DATA_INT__POS) | (1<<SMB380_LATCH_INT__POS))









/* Function prototypes */




int smb380_init(smb380_t *);

int smb380_set_image (smb380regs_t *);

int smb380_get_image(smb380regs_t *);

int smb380_get_offset(unsigned char, unsigned short *); 

int smb380_set_offset(unsigned char, unsigned short ); 

int smb380_set_offset_eeprom(unsigned char, unsigned short);

int smb380_soft_reset(void); 

int smb380_update_image(void); 

int smb380_write_ee(unsigned char , unsigned char ) ;

int smb380_set_ee_w(unsigned char);

int smb380_get_ee_w(unsigned char*);

int smb380_selftest(unsigned char);

int smb380_get_selftest_result(unsigned char *);

int smb380_set_range(char); 

int smb380_get_range(unsigned char*);

int smb380_set_mode(unsigned char); 

int smb380_get_mode(unsigned char *);

int smb380_set_wake_up_pause(unsigned char);

int smb380_get_wake_up_pause(unsigned char *);

int smb380_set_bandwidth(char);

int smb380_get_bandwidth(unsigned char *);

int smb380_set_low_g_threshold(unsigned char);

int smb380_get_low_g_threshold(unsigned char*);

int smb380_set_low_g_hysteresis(unsigned char);

int smb380_set_low_g_countdown(unsigned char);

int smb380_get_low_g_countdown(unsigned char *);

int smb380_get_low_g_hysteresis(unsigned char*);

int smb380_set_low_g_duration(unsigned char);

int smb380_get_low_g_duration(unsigned char*);

int smb380_set_high_g_threshold(unsigned char);

int smb380_get_high_g_threshold(unsigned char*);

int smb380_set_high_g_hysteresis(unsigned char);

int smb380_set_high_g_countdown(unsigned char);

int smb380_get_high_g_countdown(unsigned char *);

int smb380_get_high_g_hysteresis(unsigned char*);

int smb380_set_high_g_duration(unsigned char);

int smb380_get_high_g_duration(unsigned char*);

int smb380_set_any_motion_threshold(unsigned char);

int smb380_get_any_motion_threshold(unsigned char*);

int smb380_set_any_motion_count(unsigned char);

int smb380_get_any_motion_count(unsigned char *);

int smb380_read_accel_x(short *);

int smb380_read_accel_y(short *);

int smb380_read_accel_z(short *);

int smb380_read_temperature(unsigned char*);

int smb380_read_accel_xyz(smb380acc_t *);

int smb380_get_interrupt_status(unsigned char *);

int smb380_reset_interrupt(void);

int smb380_set_interrupt_mask(unsigned char);

int smb380_get_interrupt_mask(unsigned char *);

int smb380_set_low_g_int(unsigned char);

int smb380_set_high_g_int(unsigned char);

int smb380_set_any_motion_int(unsigned char);

int smb380_set_alert_int(unsigned char);

int smb380_set_advanced_int(unsigned char);

int smb380_latch_int(unsigned char);

int smb380_set_new_data_int(unsigned char onoff);

int smb380_pause(int);

int smb380_read_reg(unsigned char , unsigned char *, unsigned char);

int smb380_write_reg(unsigned char , unsigned char*, unsigned char );




#endif   // __SMB380_H__





