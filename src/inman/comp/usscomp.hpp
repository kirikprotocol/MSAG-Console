#ident "$Id$"
#ifndef __SMSC_INMAN_MAPUSS_COMPS_HPP__
#define __SMSC_INMAN_MAPUSS_COMPS_HPP__

#include "inman/common/adrutil.hpp"
#include "logger/Logger.h"
#include "compdefs.hpp"
#include "acdefs.hpp"

using smsc::logger::Logger;
using smsc::inman::comp::Component;
using smsc::inman::comp::OperationFactory;

#define MAP_MAX_USSD_StringLength   160 //encoded or GSM 7bit packed
#define MAX_USSD_TEXT8_LENGTH        90
#define MAX_USSD_TEXT16_LENGTH       40


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


//The base class for MAP USS2 requests/results
class MAPUSS2Comp : public Component
{
public:
    MAPUSS2Comp() {}
    virtual ~MAPUSS2Comp() {}

    //Setters:
    //assigns USS data, that is plain LATIN1 text,
    //it will be transferred in GSM 7bit packed form. 
    void setUSSData(const unsigned char * data, unsigned size) throw(CustomException);
    //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit packed, etc)
    void setRAWUSSData(unsigned char dcs, const unsigned char * data, unsigned size);

    //Getters:
    unsigned char getDCS(void) const;
    //returns USS data 'as is', i.e. encoded according to CBS coding scheme
    const vector<unsigned char>& getUSSData(void) const;
    //if possible, converts USS data to plain LATIN1 test and returns true, otherwise - false
    bool  getUSSDataAsLatin1Text(std::string & ussStr) const;

protected:
    unsigned char	    _dCS;	// unparsed data coding scheme (CBS CS)
    vector<unsigned char>   _uSSData;	// encoded USS data string (GSM 7-bit, UCS2, etc)
};

class ProcessUSSRequestArg : public MAPUSS2Comp
{
public:
    ProcessUSSRequestArg();
    ~ProcessUSSRequestArg();

    //Setters:
    //Optional parameters
    void setAlertingPattern(enum AlertingPattern alrt);
    void setMSISDNadr(const TonNpiAddress& msadr);
    void setMSISDNadr(const char * adrStr);

    //Getters:
    //Optional parameters
    bool  msISDNadr_present(void);
    bool  msAlerting_present(void);
    //returns empty TonNpiAddress if msISDN adr absent
    const TonNpiAddress& getMSISDNadr(void) const; 
    //returns alertingNotSet if alerting absent
    enum AlertingPattern getAlertingPattern(void) const;

    void encode(vector<unsigned char>& buf) throw(CustomException);
    void decode(const vector<unsigned char>& buf) throw(CustomException);

private:
    //Optional parameters:
    AlertingPattern_e 	_alrt;		// == alertingNotSet for absence
    TonNpiAddress      	_msAdr; 	// MS ISDN address, '\0' or "0" for absence
    Logger*		compLogger;
};

class ProcessUSSRequestRes : public MAPUSS2Comp
{
public:
    ProcessUSSRequestRes();
    ~ProcessUSSRequestRes();

    void encode(vector<unsigned char>& buf) throw(CustomException);
    void decode(const vector<unsigned char>& buf) throw(CustomException);

private:
    Logger*		compLogger;
};

}//namespace usscomp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_MAPUSS_COMPS_HPP__ */
