#ifndef __EMAILSME_STATUS_CODES_H__
#define __EMAILSME_STATUS_CODES_H__

namespace smsc{
namespace emailsme{

namespace StatusCodes{

const int STATUS_CODE_OK                 =  0x0;
const int STATUS_CODE_INVALIDMSG         =  0x1;
const int STATUS_CODE_NOUSER             =  0x2;
const int STATUS_CODE_TEMPORARYERROR     =  0x3;

const int STATUS_CODE_UNKNOWNERROR       =  0x100;

};//StatusCodes
};//emailsme
};//smsc

#endif
