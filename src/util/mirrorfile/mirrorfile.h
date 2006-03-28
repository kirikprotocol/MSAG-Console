#ifndef __SMSC_UTIL_MIRRORFILE_MIRRORFILE_H__
#define __SMSC_UTIL_MIRRORFILE_MIRRORFILE_H__

extern void InitMirrorFile();

struct InitMirrorFileDummy{
  InitMirrorFileDummy()
  {
    InitMirrorFile();
  }
};
static InitMirrorFileDummy initMirrorFileDummy;

#endif
