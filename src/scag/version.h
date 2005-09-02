#ifndef __SCAG_SYSTEM_VERSION__
#define __SCAG_SYSTEM_VERSION__

#ifdef __cplusplus
extern "C" {
#endif

const char* getStrVersion();
void getVersion(int* major,int* minor,int* build);

#ifdef __cplusplus
};
#endif

#endif
