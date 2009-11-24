#ifndef __smsc_statemachine_constants_hpp__
#define __smsc_statemachine_constants_hpp__

namespace smsc{
namespace statemachine{

static const int REPORT_ACK=2;
static const int REPORT_NOACK=255;

//divert flags
static const int DF_UNCOND=1;
static const int DF_ABSENT=2;
static const int DF_BLOCK =4;
static const int DF_BARRED =8;
static const int DF_CAPAC =16;
static const int DF_COND  =DF_ABSENT|DF_BLOCK|DF_BARRED|DF_CAPAC;

static const int DF_UDHCONCAT=128;

static const int DF_DCSHIFT=8;

}
}

#endif
