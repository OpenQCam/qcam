#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/writemovdocument.h"

WriteMovDocument::WriteMovDocument(void)
{
  mp4file = NULL;
}

WriteMovDocument::~WriteMovDocument(void)
{
  if (mp4file) {
    Close();
  }

  if (mp4file) {
    QMP4_FILE_DESTRUCT(mp4file); 
  }
}

int WriteMovDocument::Open(QMP4_TRAK_INFO *vidTrakInfo,
                           QMP4_TRAK_INFO *audTrakInfo,
                           QMP4_FILE_TYPE type,
                           const char *fileName)
{
  int err = -EPERM;

  if (mp4file) {
    goto FUNC_EXIT;
  }

  QMP4_FILE_CONSTRUCT(mp4file);

  if (!mp4file) {
    err = -ENOMEM;
    goto FUNC_EXIT;
  }

  maxAllowVidPackets = vidTrakInfo->info.general.max_packets;
  maxAllowAudPackets = audTrakInfo->info.general.max_packets;
  vidPacketCount = 0;
  audPacketCount = 0;

  err = mp4file->Init(mp4file, vidTrakInfo, audTrakInfo, type, fileName);

FUNC_EXIT:

  return err;
}

int WriteMovDocument::Close(void)
{
  int err = -EPERM;

  if (!mp4file) {
    goto FUNC_EXIT;
  }

  err = mp4file->Fini(mp4file);

  if (err) {
    goto FUNC_EXIT;
  }

  QMP4_FILE_DESTRUCT(mp4file); 

FUNC_EXIT:

  return err;
}

int WriteMovDocument::UpdateSPSR(unsigned char *packetSPS,
                                 unsigned int sizeSPS,
                                 unsigned char *packetPPS,
                                 unsigned int sizePPS)
{
  int err = -EPERM;

  if (!mp4file) {
    goto FUNC_EXIT;
  }

  err = mp4file->UpdateSPSR(mp4file, packetSPS, sizeSPS, packetPPS, sizePPS);

FUNC_EXIT:

  return err;
}

int WriteMovDocument::AddVidData(unsigned char *packet,
                                 unsigned int size,
                                 unsigned int delta,
                                 unsigned int isSyncFrame)
{
  int err = -EPERM;

  if (!mp4file) {
    goto FUNC_EXIT;
  }

  err = mp4file->AddVidData(mp4file, packet, size, delta, isSyncFrame);

  err = err ? err : (((++vidPacketCount) == maxAllowVidPackets) ? QMP4_SUCCESS_DATA_FULL : QMP4_SUCCESS);

FUNC_EXIT:

  return err;
}

int WriteMovDocument::AddAudData(unsigned char *packet,
                                 unsigned int size,
                                 unsigned int sampleNums)

{
  int err = -EPERM;

  if (!mp4file) {
    goto FUNC_EXIT;
  }

  err = mp4file->AddAudData(mp4file, packet, size, sampleNums);

  err = err ? err : (((++audPacketCount) == maxAllowAudPackets) ? QMP4_SUCCESS_DATA_FULL : QMP4_SUCCESS);

FUNC_EXIT:

  return err;
}

