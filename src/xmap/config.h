#ifndef __CONFIG_HHHH___
#define __CONFIG_HHHH___

 
#include "../src/util/xml/DOMTreeReader.h"
#include "../src/util/xml/utilFunctions.h"
#include "../src/util/xml/SmscTranscoder.h"
#include "../src/util/xml/init.h"
#include <xercesc/util/TransENameMap.hpp>
#include <xercesc/internal/IANAEncodings.hpp>
#ifdef WIN32
#include <xercesc/dom/DOMElement.hpp>
#endif
#include <string>
#include <vector>

#include <core/synchronization/EventMonitor.hpp>

using namespace smsc::util::xml;
using namespace smsc::core::synchronization;
using namespace std;

typedef unsigned short USHORT_T;
/*

class cpipe
{
public:
 cpipe();
 cpipe(const cpipe & cp);
 cpipe operator = (cpipe & other_);
 ~cpipe();
public:
 std::string originating_addr;
 std::string destination_addr;
 std::string message_text;
 std::string imsi;
 USHORT_T random_case;
 bool sender;
 
};
*/

class ConfigXml
{
public:
 ConfigXml();
 ~ConfigXml();
 
 int readXMLConfig(const char* filename);
 
 USHORT_T getSmscSSN();
 USHORT_T getUSSDSSN();
 USHORT_T getTimeouts();
 bool isAutostart();
 
 
 long getMqueueLength();
 std::string getMscAddr();

 std::string getSmeAddress();
 std::string getSmeHost();
 int getSmePort();
 std::string getSmeSid();

// std::vector<cpipe> vpipes;


private:
 int readTimeouts(DOMElement *docElem);
 int readMscAddr(DOMElement *docElem);
 int readSmes(DOMElement *docElem);
 

 long getIntValAttr(DOMNode* node,const char * attr_name);
 std::string getStringValAttr(DOMNode* node,const char * attr_name);

 USHORT_T timeout;
 USHORT_T smsc_ssn;
 USHORT_T ussd_ssn;
 
 long mqueuelen;
 bool autostart;
 std::string str_mscaddr;
 EventMonitor mtx;
 std::string sme_address;
 std::string sme_sid;
 std::string sme_host;
 int sme_port;
 
};


#endif