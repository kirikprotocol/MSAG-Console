#ident "$Id$"
#ifndef __SMSC_INMAN_MAPUSS_COMPS_HPP__
#define __SMSC_INMAN_MAPUSS_COMPS_HPP__

#include <vector>
#include <stdexcept>
#include "sms/sms.h"
#include "logger/Logger.h"
#include "compdefs.hpp"
#include "acdefs.hpp"
//#include "operfactory.hpp"

using smsc::logger::Logger;

using std::runtime_error;
using std::vector;
using smsc::sms::Address;
using smsc::inman::comp::Component;
using smsc::inman::comp::OperationFactory;

#define MAP_MAX_USSD_StringLength 160

namespace smsc {
namespace inman {
namespace usscomp {

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


struct MAPUSS_OpCode {
    enum {
	processUSS_Request = 59
    //  ,USS_Request = 60
    //  ,USS-Notify = 61
    };
};

struct ERR_ProcessUSS_Request {
    enum {
        callBarred = 13, systemFailure = 34, dataMissing = 35,
        unexpectedDataValue = 36, unknownAlphabet = 71
    };
};
//FactoryInitFunction declaration
OperationFactory * initMAPUSS2Components(OperationFactory * fact);


class ProcessUSSRequestArg : public Component
{
public:
    ProcessUSSRequestArg();
    ~ProcessUSSRequestArg();

    //Setters:
    void setDCS(unsigned char dcs);
    void setUSSData(unsigned char * data, unsigned size);
    //Optional parameters
    void setAlertingPattern(enum AlertingPattern alrt);
    void setMSISDNadr(const Address& msadr);
    void setMSISDNadr(const char * adrStr);

    //Getters:
    unsigned char getDCS(void) const;
    const vector<unsigned char>& getUSSData(void) const;
    //Optional parameters
    bool  msISDNadr_present(void);
    bool  msAlerting_present(void);
    //returns empty Address if msISDN adr absent
    const Address& getMSISDNadr(void) const; 
    //returns alertingNotSet if alerting absent
    enum AlertingPattern getAlertingPattern(void) const;

    void encode(vector<unsigned char>& buf);
    void decode(const vector<unsigned char>& buf);

private:
    unsigned char	    _dCS;	// unparsed data coding scheme
    vector<unsigned char>   _uSSdata;	// USS data string:  GSM 7-bit or UCS2

    //Optional parameters:
    AlertingPattern_e 	_alrt;		// == alertingNotSet for absence
    Address 		_msAdr; 	// MS ISDN address, '\0' or "0" for absence

    Logger*		compLogger;
};

class ProcessUSSRequestRes : public Component
{
public:
    ProcessUSSRequestRes();
    ~ProcessUSSRequestRes();

    //Setters:
    void setDCS(unsigned char dcs);
    void setUSSData(unsigned char * data, unsigned size);
    //Getters:
    unsigned char getDCS(void) const;
    const vector<unsigned char>& getUSSData(void) const;

    void encode(vector<unsigned char>& buf);
    void decode(const vector<unsigned char>& buf);

private:
    unsigned char	    _dCS;	// unparsed data coding scheme
    vector<unsigned char>   _uSSdata;	// USS data string:  GSM 7-bit or UCS2

    Logger*		compLogger;
};

}//namespace usscomp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_MAPUSS_COMPS_HPP__ */
