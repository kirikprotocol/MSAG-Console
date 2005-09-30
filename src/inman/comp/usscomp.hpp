#ident "$Id$"
#ifndef __SMSC_INMAN_MAPUSS_COMPS_HPP__
#define __SMSC_INMAN_MAPUSS_COMPS_HPP__

#include <vector>
#include <stdexcept>
#include "sms/sms.h"
#include "logger/Logger.h"
#include "comps.hpp"

using smsc::logger::Logger;

using std::runtime_error;
using std::vector;
using smsc::sms::Address;
using smsc::inman::comp::Component;

#define MAP_MAX_USSD_StringLength 160

namespace smsc {
namespace mapuss {
namespace comp {

// Alerting categories are defined in GSM 02.07
typedef enum AlertingPattern {
    alertingLevel_0 =		0,
    alertingLevel_1 =		1,
    alertingLevel_2 =		2,
    alertingCategory_1 =	4,
    alertingCategory_2 =	5,
    alertingCategory_3 =	6,
    alertingCategory_4 =	7,
    alertingCategory_5 =	8,
    alertingNotSet =		128	//0x80
} AlertingPattern_e;

typedef enum USS_OperationCode {
    processUSS_Request = 59
//  ,USS_Request = 60
//  ,USS-Notify = 61
} USS_OperationCode_e;


class ProcessUSSRequestArg : public Component
{
public:
    ProcessUSSRequestArg();
    ~ProcessUSSRequestArg();

    unsigned char	    _dCS;	// unparsed data coding scheme
    vector<unsigned char>   _uSSdata;	// USS data string:  GSM 7-bit or UCS2

    //Optional parameters:
    AlertingPattern_e 	_alrt;		// == alertingNotSet for absence
    Address 		_msAdr; 	// MS ISDN address, '\0' or "0" for absence

    void setUSSData(unsigned char * data, unsigned size);
    //Optional parameters
    void setAlertingPattern(enum AlertingPattern alrt);
    void setMSISDNadr(const Address& msadr);

    //some usefull methods:
    const Address& getMSISDNadr(void); 
    const vector<unsigned char>& getUSSData(void);
    //checks wether the MSisdnAdr is set or not, returns TRUE/FALSE
    int   msISDNadr_present(void); 

    void encode(vector<unsigned char>& buf);
    void decode(const vector<unsigned char>& buf);
private:
    Logger*		compLogger;
};

class ProcessUSSRequestRes : public Component
{
public:
    ProcessUSSRequestRes();
    ~ProcessUSSRequestRes();

    unsigned char	    _dCS;	// unparsed data coding scheme
    vector<unsigned char>   _uSSdata;	// USS data string:  GSM 7-bit or UCS2

    void setUSSData(unsigned char * data, unsigned size);
    const vector<unsigned char>& getUSSData(void);

    void encode(vector<unsigned char>& buf);
    void decode(const vector<unsigned char>& buf);
private:
    Logger*		compLogger;
};

}//namespace comp
}//namespace mapuss
}//namespace smsc

#endif /* __SMSC_INMAN_MAPUSS_COMPS_HPP__ */
