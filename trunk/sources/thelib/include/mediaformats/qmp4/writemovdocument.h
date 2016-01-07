#ifndef __WRITEMOVDOCUMENT_H__
#define __WRITEMOVDOCUMENT_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qmp4file.h"

#define QMP4_SUCCESS            0
#define QMP4_SUCCESS_DATA_FULL  1

class WriteMovDocument {

private:
  QMP4_FILE *mp4file;
  unsigned int maxAllowVidPackets;
  unsigned int maxAllowAudPackets;
  unsigned int vidPacketCount;
  unsigned int audPacketCount;

public:
  WriteMovDocument(void);
  ~WriteMovDocument(void);

  int Open(QMP4_TRAK_INFO *vidTrakInfo,
           QMP4_TRAK_INFO *audTrakInfo,
           QMP4_FILE_TYPE type,
           const char *fileName);  
  int Close(void);

  int UpdateSPSR(unsigned char *packetSPS,
                 unsigned int sizeSPS,
                 unsigned char *packetPPS,
                 unsigned int sizePPS);
  int AddVidData(unsigned char *packet,
                 unsigned int size,
                 unsigned int delta,
                 unsigned int isSyncFrame);
  int AddAudData(unsigned char *packet,
                 unsigned int size,
                 unsigned int sampleNums);
};

#endif /* __WRITEMOVDOCUMENT_H__ */
