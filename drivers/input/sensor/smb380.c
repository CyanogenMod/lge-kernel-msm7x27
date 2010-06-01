/*  $Date: 2008/06/03 16:22:35 $
 *  $Revision: 1.8 $
 *  
 */

/*
* Copyright (C) 2007 Bosch Sensortec GmbH
*
* SMB380 acceleration sensor API
* 
* Usage:	Application Programming Interface for SMB380 configuration and data read out
*
* Author:	Lars.Beseke@bosch-sensortec.com
*
* Disclaimer
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


/*! \file smb380.c
    \brief This file contains all function implementations for the SMB380 API
    
    Details.
*/


#include "smb380.h"


smb380_t *p_smb380;				/**< pointer to SMB380 device structure  */


/** API Initialization routine
 \param *smb380 pointer to SMB380 structured type
 \return result of communication routines 
 */

int smb380_init(smb380_t *smb380) 
{
	int comres=0;
	unsigned char data;

	p_smb380 = smb380;																			/* assign smb380 ptr */
	p_smb380->dev_addr = SMB380_I2C_ADDR;										/* preset SM380 I2C_addr */
	comres += p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_CHIP_ID__REG, &data, 1);	/* read Chip Id */
	
	p_smb380->chip_id = SMB380_GET_BITSLICE(data, SMB380_CHIP_ID);						/* get bitslice */
		
	comres += p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ML_VERSION__REG, &data, 1); /* read Version reg */
	p_smb380->ml_version = SMB380_GET_BITSLICE(data, SMB380_ML_VERSION);				/* get ML Version */
	p_smb380->al_version = SMB380_GET_BITSLICE(data, SMB380_AL_VERSION);				/* get AL Version */

	return comres;

}

/** Perform soft reset of SMB380 via bus command
*/
int smb380_soft_reset() 
{
	int comres;
	unsigned char data=0;
	if (p_smb380==0) 
		return E_SMB_NULL_PTR;
	data = SMB380_SET_BITSLICE(data, SMB380_SOFT_RESET, 1);
  comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_SOFT_RESET__REG, &data,1); 
	return comres;
}


/** call SMB380s update image function
	\return bus communication result
*/
int smb380_update_image() 
{
	int comres;
	unsigned char data=0;
	if (p_smb380==0) 
		return E_SMB_NULL_PTR;
	data = SMB380_SET_BITSLICE(data, SMB380_UPDATE_IMAGE, 1);
    comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_UPDATE_IMAGE__REG, &data,1); 
	return comres;
}


/** copy image from image structure to SMB380 image memory
   \param smb380Image Pointer to smb380regs_t
   \return result of bus communication function
*/
int smb380_set_image (smb380regs_t *smb380Image) 
{
	int comres;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
    comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG,&data, 1);
	data = SMB380_SET_BITSLICE(data, SMB380_EE_W, SMB380_EE_W_ON);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG, &data, 1);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_IMAGE_BASE, (unsigned char*)smb380Image, SMB380_IMAGE_LEN);
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG,&data, 1);
	data = SMB380_SET_BITSLICE(data, SMB380_EE_W, SMB380_EE_W_OFF);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG, &data, 1);
	return comres;
}



/** read out image from SMB380 and store it to smb380regs_t structure
   \param smb380Image pointer to smb380regs_t 
   \return result of bus communication function

*/
int smb380_get_image(smb380regs_t *smb380Image)
{

	int comres;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
        comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG,&data, 1);
	data = SMB380_SET_BITSLICE(data, SMB380_EE_W, SMB380_EE_W_ON);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG, &data, 1);
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_IMAGE_BASE, (unsigned char *)smb380Image, SMB380_IMAGE_LEN);
	data = SMB380_SET_BITSLICE(data, SMB380_EE_W, SMB380_EE_W_OFF);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG, &data, 1);
	return comres;
}

/** read out offset data from 
   \param xyz select axis x=0, y=1, z=2
   \param *offset pointer to offset value (offset is in offset binary representation
   \return result of bus communication function
   \note use smb380_set_ee_w() function to enable access to offset registers 
*/
int smb380_get_offset(unsigned char xyz, unsigned short *offset) 
{

   int comres;
   unsigned char data;
   if (p_smb380==0)
   		return E_SMB_NULL_PTR;
   comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_LSB__REG+xyz), &data, 1);
   data = SMB380_GET_BITSLICE(data, SMB380_OFFSET_X_LSB);
   *offset = data;
   comres += p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_MSB__REG+xyz), &data, 1);
   *offset |= (data<<2);
   return comres;
}


/** write offset data to SMB380 image
   \param xyz select axis x=0, y=1, z=2
   \param offset value to write (offset is in offset binary representation
   \return result of bus communication function
   \note use smb380_set_ee_w() function to enable access to offset registers 
*/
int smb380_set_offset(unsigned char xyz, unsigned short offset) 
{

   int comres;
   unsigned char data;
   if (p_smb380==0)
   		return E_SMB_NULL_PTR;
   comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_LSB__REG+xyz), &data, 1);
   data = SMB380_SET_BITSLICE(data, SMB380_OFFSET_X_LSB, offset);
   comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_LSB__REG+xyz), &data, 1);
   data = (offset&0x3ff)>>2;
   comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_MSB__REG+xyz), &data, 1);
   return comres;
}


/** write offset data to SMB380 image
   \param xyz select axis x=0, y=1, z=2
   \param offset value to write to eeprom(offset is in offset binary representation
   \return result of bus communication function
   \note use smb380_set_ee_w() function to enable access to offset registers in EEPROM space
*/
int smb380_set_offset_eeprom(unsigned char xyz, unsigned short offset) 
{

   int comres;
   unsigned char data;
   if (p_smb380==0)
   		return E_SMB_NULL_PTR;   
   comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, (SMB380_OFFSET_X_LSB__REG+xyz), &data, 1);
   data = SMB380_SET_BITSLICE(data, SMB380_OFFSET_X_LSB, offset);
   comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, (SMB380_EEP_OFFSET+SMB380_OFFSET_X_LSB__REG + xyz), &data, 1);   
   p_smb380->delay_msec(34);
   data = (offset&0x3ff)>>2;
   comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, (SMB380_EEP_OFFSET+ SMB380_OFFSET_X_MSB__REG+xyz), &data, 1);
   p_smb380->delay_msec(34);
   return comres;
}




/** write offset data to SMB380 image
   \param eew 0 = lock EEPROM 1 = unlock EEPROM 
   \return result of bus communication function
*/
int smb380_set_ee_w(unsigned char eew)
{
  unsigned char data;
	int comres;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
  comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG,&data, 1);
	data = SMB380_SET_BITSLICE(data, SMB380_EE_W, eew);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG, &data, 1);
	return comres;
}

/** get eew-flag
   \param *eew pointer to eew-flag
   \return result of bus communication function
*/
int smb380_get_ee_w(unsigned char *eew)
{
	int comres;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
  comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EE_W__REG,eew, 1);
	*eew = SMB380_GET_BITSLICE(*eew, SMB380_EE_W);
	return comres;
}



/** write byte to SMB380 EEPROM
   \param addr address to write to (image addresses are automatically extended to EEPROM space)
   \param data byte content to write 
   \return result of bus communication function
*/
int smb380_write_ee(unsigned char addr, unsigned char data) 
{	
	int comres;
	if (p_smb380==0) 			/* check pointers */
		return E_SMB_NULL_PTR;
    if (p_smb380->delay_msec == 0)
	    return E_SMB_NULL_PTR;
    comres = smb380_set_ee_w( SMB380_EE_W_ON );
	addr|=0x20;   /* add eeprom address offset to image address if not applied */
	comres += smb380_write_reg(addr, &data, 1 );
	p_smb380->delay_msec( SMB380_EE_W_DELAY );
	comres += smb380_set_ee_w( SMB380_EE_W_OFF);
	return comres;
}

/**	start SMB380s integrated selftest function
   \param st 1 = selftest0, 2 = selftest1 (see also)
 	 \return result of bus communication function
 	 
 	 \see SMB380_SELF_TEST0_ON
 	 \see SMB380_SELF_TEST1_ON
 
 */
int smb380_selftest(unsigned char st)
{
	int comres;
	unsigned char data;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_SELF_TEST__REG, &data, 1);
	data = SMB380_SET_BITSLICE(data, SMB380_SELF_TEST, st);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_SELF_TEST__REG, &data, 1);  
	return comres;  

}



/**	set smb380s range 
 \param range 
 \return  result of bus communication function
 
 \see SMB380_RANGE_2G		
 \see SMB380_RANGE_4G			
 \see SMB380_RANGE_8G			
*/
int smb380_set_range(char range) 
{			
   int comres = 0;
   unsigned char data;

   if (p_smb380==0)
	    return E_SMB_NULL_PTR;

   if (range<3) {	
	 	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_RANGE__REG, &data, 1 );
	 	data = SMB380_SET_BITSLICE(data, SMB380_RANGE, range);		  	
         comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_RANGE__REG, &data, 1);
   }
   return comres;

}


/* readout select range from SMB380 
   \param *range pointer to range setting
   \return result of bus communication function
   \see SMB380_RANGE_2G, SMB380_RANGE_4G, SMB380_RANGE_8G		
   \see smb380_set_range()
*/
int smb380_get_range(unsigned char *range) 
{

	int comres = 0;
	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_RANGE__REG, range, 1 );

	*range = SMB380_GET_BITSLICE(*range, SMB380_RANGE);
	
	return comres;

}



/** set SMB380s operation mode
   \param mode 0 = normal, 2 = sleep, 3 = auto wake up
   \return result of bus communication function
   \note Available constants see below
   \see SMB380_MODE_NORMAL, SMB380_MODE_SLEEP, SMB380_MODE_WAKE_UP     
	 \see smb380_get_mode()
*/
int smb380_set_mode(unsigned char mode) {
	
	int comres=0;
	unsigned char data1, data2;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	if (mode<4 && mode!=1) {
		comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_WAKE_UP__REG, &data1, 1 );
		data1  = SMB380_SET_BITSLICE(data1, SMB380_WAKE_UP, mode);		  
        comres += p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_SLEEP__REG, &data2, 1 );
		data2  = SMB380_SET_BITSLICE(data2, SMB380_SLEEP, (mode>>1));
    	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_WAKE_UP__REG, &data1, 1);
	  	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_SLEEP__REG, &data2, 1);
	  	p_smb380->mode = mode;
	} 
	return comres;
	
}



/** get selected mode
   \return used mode
   \note this function returns the mode stored in \ref smb380_t structure
   \see SMB380_MODE_NORMAL, SMB380_MODE_SLEEP, SMB380_MODE_WAKE_UP
   \see smb380_set_mode()

*/
int smb380_get_mode(unsigned char *mode) 
{
    if (p_smb380==0)
    	return E_SMB_NULL_PTR;	
		*mode =  p_smb380->mode;
	  return 0;
}

/** set SMB380 internal filter bandwidth
   \param bw bandwidth (see bandwidth constants)
   \return result of bus communication function
   \see #define SMB380_BW_25HZ, SMB380_BW_50HZ, SMB380_BW_100HZ, SMB380_BW_190HZ, SMB380_BW_375HZ, SMB380_BW_750HZ, SMB380_BW_1500HZ
   \see smb380_get_bandwidth()
*/
int smb380_set_bandwidth(char bw) 
{
	int comres = 0;
	unsigned char data;


	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	if (bw<8) {

  	  comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_BANDWIDTH__REG, &data, 1 );
	  data = SMB380_SET_BITSLICE(data, SMB380_BANDWIDTH, bw);
	  comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_BANDWIDTH__REG, &data, 1 );

	}
    return comres;


}

/** read selected bandwidth from SMB380 
 \param *bw pointer to bandwidth return value
 \return result of bus communication function
 \see #define SMB380_BW_25HZ, SMB380_BW_50HZ, SMB380_BW_100HZ, SMB380_BW_190HZ, SMB380_BW_375HZ, SMB380_BW_750HZ, SMB380_BW_1500HZ
 \see smb380_set_bandwidth()
*/
int smb380_get_bandwidth(unsigned char *bw) {
	int comres = 1;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_BANDWIDTH__REG, bw, 1 );		

	*bw = SMB380_GET_BITSLICE(*bw, SMB380_BANDWIDTH);
	
	return comres;

}

/** set SMB380 auto wake up pause
  \param wup wake_up_pause parameters
	\return result of bus communication function
	\see SMB380_WAKE_UP_PAUSE_20MS, SMB380_WAKE_UP_PAUSE_80MS, SMB380_WAKE_UP_PAUSE_320MS, SMB380_WAKE_UP_PAUSE_2560MS
	\see smb380_get_wake_up_pause()
*/

int smb380_set_wake_up_pause(unsigned char wup)
{
	int comres=0;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;


	    comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_WAKE_UP_PAUSE__REG, &data, 1 );
		data = SMB380_SET_BITSLICE(data, SMB380_WAKE_UP_PAUSE, wup);
		comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_WAKE_UP_PAUSE__REG, &data, 1 );
	return comres;
}

/** read SMB380 auto wake up pause from image
  \param *wup wake up pause read back pointer
	\see SMB380_WAKE_UP_PAUSE_20MS, SMB380_WAKE_UP_PAUSE_80MS, SMB380_WAKE_UP_PAUSE_320MS, SMB380_WAKE_UP_PAUSE_2560MS
	\see smb380_set_wake_up_pause()
*/
int smb380_get_wake_up_pause(unsigned char *wup)
{
    int comres = 1;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_WAKE_UP_PAUSE__REG, &data,  1 );		
	
	*wup = SMB380_GET_BITSLICE(data, SMB380_WAKE_UP_PAUSE);
	
	return comres;

}


/* Thresholds and Interrupt Configuration */


/** set low-g interrupt threshold
   \param th set the threshold
   \note the threshold depends on configured range. A macro \ref SMB380_LG_THRES_IN_G() for range to register value conversion is available.
   \see SMB380_LG_THRES_IN_G()   
   \see smb380_get_low_g_threshold()
*/
int smb380_set_low_g_threshold(unsigned char th) 
{

	int comres;	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;		

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_LG_THRES__REG, &th, 1);
	return comres;
	
}


/** get low-g interrupt threshold
   \param *th get the threshold  value from sensor image
   \see smb380_set_low_g_threshold()
*/
int smb380_get_low_g_threshold(unsigned char *th)
{

	int comres=1;	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;	

		comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_LG_THRES__REG, th, 1);		

	return comres;

}


/** set low-g interrupt countdown
   \param cnt get the countdown value from sensor image
   \see smb380_get_low_g_countdown()
*/
int smb380_set_low_g_countdown(unsigned char cnt)
{
	int comres=0;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;
  comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_COUNTER_LG__REG, &data, 1 );
  data = SMB380_SET_BITSLICE(data, SMB380_COUNTER_LG, cnt);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_COUNTER_LG__REG, &data, 1 );
	return comres;
}

/** set low-g hysteresis 
   \param hyst sets the hysteresis value    
*/
int smb380_set_low_g_hysteresis(unsigned char hyst) 
{
int comres=0;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;
  	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_LG_HYST__REG, &data, 1 );
  	data = SMB380_SET_BITSLICE(data, SMB380_LG_HYST, hyst);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_LG_HYST__REG, &data, 1 );
	return comres;
}

/** get low-g hysteresis value
   \param hyst gets the hysteresis value from sensor
   \see smb380_set_low_g_hysteresis()
*/
int smb380_get_low_g_hysteresis(unsigned char *hyst)
{
    int comres = 0;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_LG_HYST__REG, &data,  1 );		
	
	*hyst = SMB380_GET_BITSLICE(data, SMB380_LG_HYST);
	
	return comres;

}



/** get low-g interrupt countdown
   \param cnt get the countdown  value from sensor image
   \see smb380_set_low_g_countdown()
*/
int smb380_get_low_g_countdown(unsigned char *cnt)
{
    int comres = 1;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_COUNTER_LG__REG, &data,  1 );		
	*cnt = SMB380_GET_BITSLICE(data, SMB380_COUNTER_LG);
	
	return comres;
}

/** set high-g interrupt countdown
   \param cnt get the countdown value from sensor image
   \see smb380_get_high_g_countdown()
*/
int smb380_set_high_g_countdown(unsigned char cnt)
{
	int comres=1;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;


        comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_COUNTER_HG__REG, &data, 1 );
	data = SMB380_SET_BITSLICE(data, SMB380_COUNTER_HG, cnt);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_COUNTER_HG__REG, &data, 1 );
	return comres;
}

/** get high-g interrupt countdown
   \param cnt get the countdown  value from sensor image
   \see smb380_set_high_g_countdown()
*/
int smb380_get_high_g_countdown(unsigned char *cnt)
{
    int comres = 0;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_COUNTER_HG__REG, &data,  1 );		
	
	*cnt = SMB380_GET_BITSLICE(data, SMB380_COUNTER_HG);
	
	return comres;

}


/** configure low-g duration value
	\param dur low-g duration in miliseconds
	\see smb380_get_low_g_duration(), smb380_get_high_g_duration(), smb380_set_high_g_duration()
	
*/
int smb380_set_low_g_duration(unsigned char dur) 
{
	int comres=0;	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	
	
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_LG_DUR__REG, &dur, 1);

	return comres;
}

/** set high-g hysteresis 
   \param hyst sets the hysteresis value    
*/
int smb380_set_high_g_hysteresis(unsigned char hyst) 
{
int comres=0;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;
  	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_HG_HYST__REG, &data, 1 );
  	data = SMB380_SET_BITSLICE(data, SMB380_HG_HYST, hyst);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_HG_HYST__REG, &data, 1 );
	return comres;
}

/** get high-g hysteresis value
   \param hyst gets the hysteresis value from sensor
   \see smb380_set_high_g_hysteresis()
*/
int smb380_get_high_g_hysteresis(unsigned char *hyst)
{
    int comres = 0;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_HG_HYST__REG, &data,  1 );		
	
	*hyst = SMB380_GET_BITSLICE(data, SMB380_HG_HYST);
	
	return comres;

}




/** read out low-g duration value from sensor image
	\param dur low-g duration in miliseconds
	\see smb380_set_low_g_duration(), smb380_get_high_g_duration(), smb380_set_high_g_duration()
	
*/
int smb380_get_low_g_duration(unsigned char *dur) {
	
	int comres=0;	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;



	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_LG_DUR__REG, dur, 1);				  
	return comres;

}




/** set low-g interrupt threshold
   \param th set the threshold
   \note the threshold depends on configured range. A macro \ref SMB380_HG_THRES_IN_G() for range to register value conversion is available.
   \see SMB380_HG_THRES_IN_G()   
   \see smb380_get_high_g_threshold()
*/
int smb380_set_high_g_threshold(unsigned char th) 
{

	int comres=0;	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_HG_THRES__REG, &th, 1);
	return comres;
	
}

/** get high-g interrupt threshold
   \param *th get the threshold  value from sensor image
   \see smb380_set_high_g_threshold()
*/
int smb380_get_high_g_threshold(unsigned char *th)
{

	int comres=0;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_HG_THRES__REG, th, 1);		

	return comres;

}



/** configure high-g duration value
	\param dur high-g duration in miliseconds
	\see  smb380_get_high_g_duration(), smb380_set_low_g_duration(), smb380_get_low_g_duration()
	
*/
int smb380_set_high_g_duration(unsigned char dur) 
{
	int comres=0;	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_HG_DUR__REG, &dur, 1);
	return comres;
}


/** read out high-g duration value from sensor image
	\param dur high-g duration in miliseconds
	\see  smb380_set_high_g_duration(), smb380_get_low_g_duration(), smb380_set_low_g_duration(),
	
*/
int smb380_get_high_g_duration(unsigned char *dur) {	
	
	int comres=0;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
			
        comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_HG_DUR__REG, dur, 1);		

	return comres;

}




/**  set threshold value for any_motion feature
		\param th set the threshold a macro \ref SMB380_ANY_MOTION_THRES_IN_G()  is available for that
		\see SMB380_ANY_MOTION_THRES_IN_G()
*/
int smb380_set_any_motion_threshold(unsigned char th) 
{
	int comres=0;	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ANY_MOTION_THRES__REG, &th, 1);

	return comres;
}


/**  get threshold value for any_motion feature
		\param *th read back any_motion threshold from image register 
		\see SMB380_ANY_MOTION_THRES_IN_G()
*/
int smb380_get_any_motion_threshold(unsigned char *th)
{

	int comres=0;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ANY_MOTION_THRES__REG, th, 1);		

	return comres;

}

/**  set counter value for any_motion feature 
		\param amc set the counter value, constants are available for that
		\see SMB380_ANY_MOTION_DUR_1, SMB380_ANY_MOTION_DUR_3, SMB380_ANY_MOTION_DUR_5, SMB380_ANY_MOTION_DUR_7
*/
int smb380_set_any_motion_count(unsigned char amc)
{
	int comres=0;	
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

 	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ANY_MOTION_DUR__REG, &data, 1 );
	data = SMB380_SET_BITSLICE(data, SMB380_ANY_MOTION_DUR, amc);
	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ANY_MOTION_DUR__REG, &data, 1 );
	return comres;
}


/**  get counter value for any_motion feature from image register
		\param *amc readback pointer for counter value
		\see SMB380_ANY_MOTION_DUR_1, SMB380_ANY_MOTION_DUR_3, SMB380_ANY_MOTION_DUR_5, SMB380_ANY_MOTION_DUR_7
*/
int smb380_get_any_motion_count(unsigned char *amc)
{
    int comres = 0;
	unsigned char data;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ANY_MOTION_DUR__REG, &data,  1 );		
	
	*amc = SMB380_GET_BITSLICE(data, SMB380_ANY_MOTION_DUR);
	return comres;

}




/** set the interrupt mask for SMB380's interrupt features in one mask
	\param mask input for interrupt mask
	\see SMB380_INT_ALERT, SMB380_INT_ANY_MOTION, SMB380_INT_EN_ADV_INT, SMB380_INT_NEW_DATA, SMB380_INT_LATCH, SMB380_INT_HG, SMB380_INT_LG
*/
int smb380_set_interrupt_mask(unsigned char mask) 
{
	int comres=0;
	unsigned char data[4];

	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	data[0] = mask & SMB380_CONF1_INT_MSK;
	data[2] = ((mask<<1) & SMB380_CONF2_INT_MSK);		


	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_CONF1_REG, &data[1], 1);
	comres += p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_CONF2_REG, &data[3], 1);		
	data[1] &= (~SMB380_CONF1_INT_MSK);
	data[1] |= data[0];
	data[3] &=(~(SMB380_CONF2_INT_MSK));
	data[3] |= data[2];

	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_CONF1_REG, &data[1], 1);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_CONF2_REG, &data[3], 1);

	return comres;	
}

/** get the current interrupt mask settings from SMB380 image registers
	\param *mask return variable pointer for interrupt mask
	\see SMB380_INT_ALERT, SMB380_INT_ANY_MOTION, SMB380_INT_EN_ADV_INT, SMB380_INT_NEW_DATA, SMB380_INT_LATCH, SMB380_INT_HG, SMB380_INT_LG
*/
int smb380_get_interrupt_mask(unsigned char *mask) 
{
	int comres=0;
	unsigned char data;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_CONF1_REG, &data,1);
	*mask = data & SMB380_CONF1_INT_MSK;
	p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_CONF2_REG, &data,1);
	*mask = *mask | ((data & SMB380_CONF2_INT_MSK)>>1);

	return comres;
}


/** resets the SMB380 interrupt status 
		\note this feature can be used to reset a latched interrupt

*/
int smb380_reset_interrupt(void) 
{	
	int comres=0;
	unsigned char data=(1<<SMB380_RESET_INT__POS);	
	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_RESET_INT__REG, &data, 1);
	return comres;

}





/* Data Readout */

/** X-axis acceleration data readout 
	\param *a_x pointer for 16 bit 2's complement data output (LSB aligned)
*/
int smb380_read_accel_x(short *a_x) 
{
	int comres;
	unsigned char data[2];
	
	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ACC_X_LSB__REG, data, 2);
	*a_x = SMB380_GET_BITSLICE(data[0],SMB380_ACC_X_LSB) | SMB380_GET_BITSLICE(data[1],SMB380_ACC_X_MSB)<<SMB380_ACC_X_LSB__LEN;
	*a_x = *a_x << (sizeof(short)*8-(SMB380_ACC_X_LSB__LEN+SMB380_ACC_X_MSB__LEN));
	*a_x = *a_x >> (sizeof(short)*8-(SMB380_ACC_X_LSB__LEN+SMB380_ACC_X_MSB__LEN));
	return comres;
	
}



/** Y-axis acceleration data readout 
	\param *a_y pointer for 16 bit 2's complement data output (LSB aligned)
*/
int smb380_read_accel_y(short *a_y) 
{
	int comres;
	unsigned char data[2];	


	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ACC_Y_LSB__REG, data, 2);
	*a_y = SMB380_GET_BITSLICE(data[0],SMB380_ACC_Y_LSB) | SMB380_GET_BITSLICE(data[1],SMB380_ACC_Y_MSB)<<SMB380_ACC_Y_LSB__LEN;
	*a_y = *a_y << (sizeof(short)*8-(SMB380_ACC_Y_LSB__LEN+SMB380_ACC_Y_MSB__LEN));
	*a_y = *a_y >> (sizeof(short)*8-(SMB380_ACC_Y_LSB__LEN+SMB380_ACC_Y_MSB__LEN));
	return comres;
}


/** Z-axis acceleration data readout 
	\param *a_z pointer for 16 bit 2's complement data output (LSB aligned)
*/
int smb380_read_accel_z(short *a_z)
{
	int comres;
	unsigned char data[2];	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ACC_Z_LSB__REG, data, 2);
	*a_z = SMB380_GET_BITSLICE(data[0],SMB380_ACC_Z_LSB) | SMB380_GET_BITSLICE(data[1],SMB380_ACC_Z_MSB)<<SMB380_ACC_Z_LSB__LEN;
	*a_z = *a_z << (sizeof(short)*8-(SMB380_ACC_Z_LSB__LEN+SMB380_ACC_Z_MSB__LEN));
	*a_z = *a_z >> (sizeof(short)*8-(SMB380_ACC_Z_LSB__LEN+SMB380_ACC_Z_MSB__LEN));
	return comres;
}


/** 8 bit temperature data readout 
	\param *temp pointer for 8 bit temperature output (offset binary)
	\note: an output of 0 equals -30?C, 1 LSB equals 0.5?C
*/
int smb380_read_temperature(unsigned char * temp) 
{
	int comres;	

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_TEMPERATURE__REG, temp, 1);
	return comres;

}


/** X,Y and Z-axis acceleration data readout 
	\param *acc pointer to \ref smb380acc_t structure for x,y,z data readout
	\note data will be read by multi-byte protocol into a 6 byte structure 
*/
int smb380_read_accel_xyz(smb380acc_t * acc)
{
	int comres;
	unsigned char data[6];


	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ACC_X_LSB__REG, &data[0],6);
	
	acc->x = SMB380_GET_BITSLICE(data[0],SMB380_ACC_X_LSB) | (SMB380_GET_BITSLICE(data[1],SMB380_ACC_X_MSB)<<SMB380_ACC_X_LSB__LEN);
	acc->x = acc->x << (sizeof(short)*8-(SMB380_ACC_X_LSB__LEN+SMB380_ACC_X_MSB__LEN));
	acc->x = acc->x >> (sizeof(short)*8-(SMB380_ACC_X_LSB__LEN+SMB380_ACC_X_MSB__LEN));

	acc->y = SMB380_GET_BITSLICE(data[2],SMB380_ACC_Y_LSB) | (SMB380_GET_BITSLICE(data[3],SMB380_ACC_Y_MSB)<<SMB380_ACC_Y_LSB__LEN);
	acc->y = acc->y << (sizeof(short)*8-(SMB380_ACC_Y_LSB__LEN + SMB380_ACC_Y_MSB__LEN));
	acc->y = acc->y >> (sizeof(short)*8-(SMB380_ACC_Y_LSB__LEN + SMB380_ACC_Y_MSB__LEN));
	
	
	acc->z = SMB380_GET_BITSLICE(data[4],SMB380_ACC_Z_LSB); 
	acc->z |= (SMB380_GET_BITSLICE(data[5],SMB380_ACC_Z_MSB)<<SMB380_ACC_Z_LSB__LEN);
	acc->z = acc->z << (sizeof(short)*8-(SMB380_ACC_Z_LSB__LEN+SMB380_ACC_Z_MSB__LEN));
	acc->z = acc->z >> (sizeof(short)*8-(SMB380_ACC_Z_LSB__LEN+SMB380_ACC_Z_MSB__LEN));
	
	return comres;
	
}



/** check current interrupt status from interrupt status register in SMB380 image register
	\param *ist pointer to interrupt status byte
	\see SMB380_INT_STATUS_HG, SMB380_INT_STATUS_LG, SMB380_INT_STATUS_HG_LATCHED, SMB380_INT_STATUS_LG_LATCHED, SMB380_INT_STATUS_ALERT, SMB380_INT_STATUS_ST_RESULT
*/
int smb380_get_interrupt_status(unsigned char * ist) 
{

	int comres=0;	
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_STATUS_REG, ist, 1);
	return comres;
}

/** enable/ disable low-g interrupt feature
		\param onoff enable=1, disable=0
*/

int smb380_set_low_g_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ENABLE_LG__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_ENABLE_LG, onoff);
	
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ENABLE_LG__REG, &data, 1);
	
	return comres;

}

/** enable/ disable high-g interrupt feature
		\param onoff enable=1, disable=0
*/

int smb380_set_high_g_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ENABLE_HG__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_ENABLE_HG, onoff);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ENABLE_HG__REG, &data, 1);
	
	return comres;

}



/** enable/ disable any_motion interrupt feature
		\param onoff enable=1, disable=0
		\note for any_motion interrupt feature usage see also \ref smb380_set_advanced_int()
*/
int smb380_set_any_motion_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_EN_ANY_MOTION__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_EN_ANY_MOTION, onoff);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_EN_ANY_MOTION__REG, &data, 1);
	
	return comres;

}

/** enable/ disable alert-int interrupt feature
		\param onoff enable=1, disable=0
		\note for any_motion interrupt feature usage see also \ref smb380_set_advanced_int()
*/
int smb380_set_alert_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ALERT__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_ALERT, onoff);

	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ALERT__REG, &data, 1);
	
	return comres;

}


/** enable/ disable advanced interrupt feature
		\param onoff enable=1, disable=0
		\see smb380_set_any_motion_int()
		\see smb380_set_alert_int()
*/
int smb380_set_advanced_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_ENABLE_ADV_INT__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_ENABLE_ADV_INT, onoff);

	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_ENABLE_ADV_INT__REG, &data, 1);
	
	return comres;

}

/** enable/disable latched interrupt for all interrupt feature (global option)
	\param latched (=1 for latched interrupts), (=0 for unlatched interrupts) 
*/

int smb380_latch_int(unsigned char latched) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;
	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_LATCH_INT__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_LATCH_INT, latched);

	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_LATCH_INT__REG, &data, 1);
	
	return comres;

}



/** enable/ disable new data interrupt feature
		\param onoff enable=1, disable=0
*/

int smb380_set_new_data_int(unsigned char onoff) {
	int comres;
	unsigned char data;
	if(p_smb380==0) 
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, SMB380_NEW_DATA_INT__REG, &data, 1);				
	data = SMB380_SET_BITSLICE(data, SMB380_NEW_DATA_INT, onoff);
	comres += p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, SMB380_NEW_DATA_INT__REG, &data, 1);
	
	return comres;

}



/* MISC functions */


/** calls the linked wait function

		\param msec amount of mili seconds to pause
		\return number of mili seconds waited
*/

int smb380_pause(int msec) 
{
	if (p_smb380==0)
		return E_SMB_NULL_PTR;
	else
	  p_smb380->delay_msec(msec);	
	return msec;

}


/** read function for raw register access

		\param addr register address
		\param *data pointer to data array for register read back
		\param len number of bytes to be read starting from addr
	
*/

int smb380_read_reg(unsigned char addr, unsigned char *data, unsigned char len)
{

	int comres;
	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_READ_FUNC(p_smb380->dev_addr, addr, data, len);
	return comres;

}


/** write function for raw register access
		\param addr register address
		\param *data pointer to data array for register write
		\param len number of bytes to be written starting from addr	
*/
int smb380_write_reg(unsigned char addr, unsigned char *data, unsigned char len) 
{

	int comres;

	if (p_smb380==0)
		return E_SMB_NULL_PTR;

	comres = p_smb380->SMB380_BUS_WRITE_FUNC(p_smb380->dev_addr, addr, data, len);

	return comres;

}

