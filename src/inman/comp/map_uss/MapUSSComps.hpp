/* ************************************************************************* *
 * MAP Network Unstructured Supplementary Service v2 Components definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPUSS_COMPS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_MAPUSS_COMPS_HPP__

#include "logger/Logger.h"
#include "util/TonNpiAddress.hpp"

#include "inman/comp/compdefs.hpp"
#include "inman/comp/MapOpErrors.hpp"

#define MAP_MAX_USSD_StringLength   160 //encoded or GSM 7bit packed
#define MAX_USSD_TEXT8_LENGTH        90
#define MAX_USSD_TEXT16_LENGTH       40

namespace smsc {
namespace inman {
namespace comp {
namespace uss {

using smsc::logger::Logger;
using smsc::util::TonNpiAddress;

using smsc::inman::comp::Component;
using smsc::inman::comp::MAPOpErrorId;

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
        callBarred          = MAPOpErrorId::callBarred,
        systemFailure       = MAPOpErrorId::systemFailure,
        dataMissing         = MAPOpErrorId::dataMissing,
        unexpectedDataValue = MAPOpErrorId::unexpectedDataValue,
        unknownAlphabet     = MAPOpErrorId::unknownAlphabet
    };
};


//The base class for MAP USS2 requests/results
class MAPUSS2CompAC : public Component {
public:
    MAPUSS2CompAC(Logger * use_log = NULL)
        : _dCS(0), compLogger(use_log ? use_log :
                              Logger::getInstance("smsc.inman.comp.uss"))
    { }
    virtual ~MAPUSS2CompAC()
    { }

    //Setters:
    //assigns USS data, that is plain LATIN1 text,
    //it will be transferred in GSM 7bit packed form. 
    void setUSSData(const unsigned char * data, unsigned size = 0) throw(CustomException);
    //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit packed, etc)
    void setRAWUSSData(unsigned char dcs, const unsigned char * data, unsigned size);

    //Getters:
    unsigned char getDCS(void) const { return _dCS; }
    //returns USS data 'as is', i.e. encoded according to CBS coding scheme
    const std::vector<unsigned char>& getUSSData(void) const { return _uSSData; }
    //if possible, converts USS data to plain LATIN1 test and returns true, otherwise - false
    bool  getUSSDataAsLatin1Text(std::string & ussStr) const;

protected:
    unsigned char	        _dCS;	    // unparsed data coding scheme (CBS CS)
    std::vector<unsigned char>  _uSSData;   // encoded USS data string (GSM 7-bit, UCS2, etc)
    Logger*		        compLogger; // 
};

class ProcessUSSRequestArg : public MAPUSS2CompAC {
public:
    ProcessUSSRequestArg(Logger * use_log = NULL)
        : MAPUSS2CompAC(use_log), _alrt(alertingNotSet)
    { }
    ~ProcessUSSRequestArg()
    { }

    //Setters:
    //Optional parameters
    inline void setAlertingPattern(enum AlertingPattern alrt) { _alrt = alrt; }
    inline void setMSISDNadr(const TonNpiAddress & msadr) { _msAdr = msadr; }
    void setMSISDNadr(const char * adrStr) throw(CustomException);

    //Getters:
    //Optional parameters
    bool  msISDNadr_present(void) const;
    bool  msAlerting_present(void) const;
    //returns empty TonNpiAddress if msISDN adr absent
    inline const TonNpiAddress& getMSISDNadr(void) const   { return _msAdr; }
    //returns alertingNotSet if alerting absent
    inline enum AlertingPattern getAlertingPattern(void) const { return _alrt; }

    void encode(std::vector<unsigned char>& buf) const throw(CustomException);
    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    //Optional parameters:
    AlertingPattern_e 	_alrt;		// == alertingNotSet for absence
    TonNpiAddress      	_msAdr; 	// MS ISDN address, '\0' or "0" for absence
};

class ProcessUSSRequestRes : public MAPUSS2CompAC {
public:
    ProcessUSSRequestRes(Logger * use_log = NULL)
        : MAPUSS2CompAC(use_log)
    { }
    ~ProcessUSSRequestRes()
    { }

    void encode(std::vector<unsigned char>& buf) const throw(CustomException);
    void decode(const std::vector<unsigned char>& buf) throw(CustomException);
};

} //uss
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPUSS_COMPS_HPP__ */

