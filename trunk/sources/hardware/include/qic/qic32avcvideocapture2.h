/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : S.J.Hung
 *  Purpose : Video Capture Class for QIC1832
 * ==================================================================
 */

#ifdef QIC1832

#ifndef _QIC32AVCVIDEOCAPTURE2_H
#define _QIC32AVCVIDEOCAPTURE2_H

#include "qic/qic32avcvideocapture.h"


class QIC32AVCVideoCapture2
: public QIC32AVCVideoCapture
{
    int _pipe_write_fd;
    int previous_num_of_moving_objects;

  private:
    bool hadReadStreamStatus;
    bool myStatus[16];
    int  myStreamMap[16];
    bool hadAllocTemporalLayerBuffer;
    int countTemporalLayer;
    uint8_t **temporalLayerBufferCpy;
    QIC32AVCVideoCapture2(int32_t fd, int pipe_write_fd);

  public:
    virtual ~QIC32AVCVideoCapture2();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode, int pipe_write_fd);

    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();

    // BaseV4L2VideoCapture
    //virtual bool StartCapture();

    // Helper
    uint32_t AnalyzeSEIMessages(uint8_t* buffer, uint32_t size, uint32_t& sei_begin_offset);
    bool GenerateStreamMap(bool streamAvailable[], int *streamID);
    void SendDataToCarrierAndSubStream(int rawStreamID,int temporalID,
        uint8_t *pData, uint32_t dataLength, uint64_t ts, uint32_t type);
};
#endif /* _QIC32AVCVIDEOCAPTURE2_H */

#endif

