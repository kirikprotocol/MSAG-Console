#ifndef SMS_DECLARATIONS
#define SMS_DECLARATIONS
 
#include <inttypes.h>

typedef uint32_t DateTime;

#define MAX_ADDRESS_VALUE_LENGTH    10
#define MAX_SMS_ID_VALUE_LENGTH     21
#define MAX_SHORT_MESSAGE_LENGTH    256

namespace smsc { namespace sms
{
   typedef uint8_t  AddressValue[MAX_ADDRESS_VALUE_LENGTH];
   typedef uint8_t  SMSData[MAX_SHORT_MESSAGE_LENGTH];
   typedef uint32_t SMSId;

   struct Address
   {
       uint8_t len, type, plan;
       AddressValue value;

       Address() {};
       Address(uint8_t _len, uint8_t _type, uint8_t _plan,
                sms::AddressValue& _value):
                    len(_len), type(_type), plan(_plan) { /* ... */ }
       Address(uint8_t _len, uint8_t* _buff): len(_len) { /* ... */ }
   };
   
   typedef enum {
       ENROUTE, DELIVERED, 
       EXPIRED, UNDELIVERABLE, 
       DELETED
   } State;
   
   struct SMS 
   {
       SMSId       ID;             // ??? Global message id

       State       ST;             // Message state
       Address     OA;             // Originating address
       Address     DA;             // Destination address
       
       DateTime    waitTime;       // Time when to try to deliver
       DateTime    validTime;      // Time until message is valid
       DateTime    submitTime;     // Time when message arrived to SC
       DateTime    deliveryTime;   // Time of last delivery attemp was made 

       uint8_t     MR;             // Message Reference
       uint8_t     PRI;            // ??? Priority
       uint8_t     PID;            // ??? Protocol identifier

       bool        SRR;            // Status Report Requested ?
       bool        RD;             // Reject Duplicates ?

       uint8_t     FCS;            // Failure Cause
       uint8_t     DCS;            // Data Coding Scheme
       bool        UDHI;           // Is header present in UD
       SMSData     UD;             // Message body

       char* getDecodedText();
   };

}}

#endif


