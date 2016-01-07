/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/


#ifndef _QIC_CFG_H_
#define _QIC_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif 


/*support skype api*/
//#define SKYPEAPI


/*using quanta uvc driver*/
//#define QUANTA_UVC_DRIVER

/*Supported QIC chips and APIs             */
//#define QIC1802 
//#define QIC1816 
#define QIC1822



#ifdef QIC1816
/*CQIC module1  for support special moulde version */         
//#define CQIC_MOUDLE1 

#endif

/*add force for TV F/w download issue */
#ifdef QUANTA_UVC_DRIVER
#define ADD_FORCE
#endif


/*define for MPEG AVC function*/
//#define QIC_MPEGTS_API

#ifdef QIC1822
/*define for 1822 VP8 and AVC simulcast function */
#define QIC_SIMULCAST_API  
//#define QIC_OSD_API
#define QIC_MD_API

/*support 2nd Boot Rom for FW upgarde procedure*/
//#define QIC_SUPPORT_2ND_BL

/*support backup audio calibration data*/
//#define QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP
#endif




#ifdef __cplusplus
}
#endif 

#endif
