#ifndef SMS_DECLARATIONS
#define SMS_DECLARATIONS
 
#include <inttypes.h>
#include <time.h>
#include <string.h>

#include <util/debug.h>

namespace smsc { namespace sms
{
    const int MAX_ADDRESS_VALUE_LENGTH = 21;
    const int MAX_SHORT_MESSAGE_LENGTH = 256;
    
    typedef char		AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef uint8_t  	SMSData[MAX_SHORT_MESSAGE_LENGTH];
    typedef uint32_t 	SMSId;
    
    struct Address
    {
        uint8_t      lenght, type, plan;
        AddressValue value;
        
        Address() : lenght(0), type(0), plan(0) {
			value[0] = '\0';
		};
        Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
            : lenght(_len), type(_type), plan(_plan) 
        { 
            setValue(_len, _value);
        };
  
        Address(const Address& addr) 
            : lenght(addr.lenght), type(addr.type), plan(addr.plan) 
        {
            setValue(addr.lenght, addr.value);   
        };

        Address& operator =(const Address& addr) 
        {
            type = addr.type; plan = addr.plan; 
            setValue(addr.lenght, addr.value);
            return (*this);
        };
       
        inline void setValue(uint8_t _len, const char* _value) 
        {
            __require__(_len<sizeof(AddressValue));
            
			memcpy(value, _value, _len*sizeof(uint8_t));
            value[lenght = _len] = '\0';
        };
        inline uint8_t getValue(char* _value) {
            memcpy(_value, value, lenght*sizeof(uint8_t));
			_value[lenght] = '\0';
            return lenght;
        }
        inline uint8_t getLenght() {
            return lenght;
        };
       
        inline void setTypeOfNumber(uint8_t _type) {
            type = _type;
        };
        inline uint8_t getTypeOfNumber() { 
            return type; 
        };
      
        inline void setNumberingPlan(uint8_t _plan) {
            plan = _plan;
        };
        inline uint8_t getNumberingPlan() {
            return plan;
        };
	};

    struct Body
    {
        bool        header;
        uint8_t     scheme;
        uint8_t     lenght;
        SMSData     data;

        Body() : header(false), lenght(0), scheme(0) {
			//memset((void *)data, 0, sizeof(data));
		};
        Body(uint8_t _len, uint8_t _scheme, bool _header, const uint8_t* _data)
            : header(_header), lenght(_len), scheme(_scheme) 
        { 
            setData(_len, _data);
        };
        
        Body(const Body& body) 
            : header(body.header), lenght(body.lenght), scheme(body.scheme)
        {
            setData(body.lenght, body.data);   
        };
        
        Body& operator =(const Body& body) 
        {
            header = body.header; scheme = body.scheme;
            setData(body.lenght, body.data);
            return (*this);
        };

        inline void setData(uint8_t _len, const uint8_t* _data) 
        {
            __require__(_len<=sizeof(SMSData));
            
			memcpy(data, _data, _len*sizeof(uint8_t));
            lenght = _len;
        };
        inline uint8_t getData(uint8_t* _data) {
            memcpy(_data, data, lenght*sizeof(uint8_t));
            return lenght;
        };
        
        inline void setCodingScheme(uint8_t _scheme) {
            scheme = _scheme;
        };
        inline uint8_t getCodingScheme() {
            return scheme;
        };
        
        inline void setHeaderIndicator(bool _header) {
            header = _header;
        };
        inline bool isHeaderIndicator() {
            return header;
        };

        char* getDecodedText();
    };
   
    typedef enum { 
		ENROUTE=0, DELIVERED=1, 
		EXPIRED=2, UNDELIVERABLE=3,
		DELETED=4 
	} State;
   
    struct SMS 
    {
        State       state;
        Address     originatingAddress;
        Address     destinationAddress;
        
        time_t      waitTime;       // Time when to try to deliver
        time_t      validTime;      // Time until message is valid
        time_t      submitTime;     // Time when message arrived to SC
        time_t      deliveryTime;   // Time of last delivery attemp was made 
        
        uint8_t     messageReference;
        uint8_t     messageIdentifier;
        
        uint8_t     priority;
        uint8_t     protocolIdentifier;
        
        bool        statusReportRequested;
        bool        rejectDuplicates;
        
        uint8_t     failureCause;
        
        Body        messageBody;    // Encoded & compressed message body

        
		SMS() : state(ENROUTE) {}; 
        
		SMS(const SMS& sms) :
			state(sms.state), 
			originatingAddress(sms.originatingAddress),
			destinationAddress(sms.destinationAddress), 
			waitTime(sms.waitTime), validTime(sms.validTime),
			submitTime(sms.submitTime), deliveryTime(sms.deliveryTime),
			messageReference(sms.messageReference), 
			messageIdentifier(sms.messageIdentifier),
			priority(sms.priority),
			protocolIdentifier(sms.protocolIdentifier),
			statusReportRequested(sms.statusReportRequested),
			rejectDuplicates(sms.rejectDuplicates),
			failureCause(sms.failureCause), 
			messageBody(sms.messageBody) 
		{};
        
        SMS& operator =(const SMS& sms) 
		{
			state = sms.state; 
			originatingAddress = sms.originatingAddress;
			destinationAddress = sms.destinationAddress;
			waitTime = sms.waitTime; validTime = sms.validTime; 
			submitTime = sms.submitTime; deliveryTime = sms.deliveryTime;
			messageReference = sms.messageReference; 
			messageIdentifier = sms.messageIdentifier;
			priority = sms.priority; 
			protocolIdentifier = sms.protocolIdentifier;
			statusReportRequested = sms.statusReportRequested;
			rejectDuplicates = sms.rejectDuplicates;
			failureCause = sms.failureCause;
			messageBody = sms.messageBody;
		};
		
		inline void setState(State state) {
            this->state = state;
        };
        inline State getState() {
            return state;
        };

        inline void setOriginatingAddress(uint8_t lenght, uint8_t type, 
                                         uint8_t plan, char* buff) 
        { // Copies address value from 'buff' to static structure
            originatingAddress.setTypeOfNumber(type);
            originatingAddress.setNumberingPlan(plan);
            originatingAddress.setValue(lenght, buff);
        };
        inline void setOriginatingAddress(Address& address) 
        { // Copies address value from 'address' to static structure 
            originatingAddress = address;     
        };
        inline Address& getOriginatingAddress() {
            return originatingAddress; 
        };

        inline void setDestinationAddress(uint8_t lenght, uint8_t type, 
                                          uint8_t plan, char* buff) 
        { // Copies address value from 'buff' to static structure 
            destinationAddress.setTypeOfNumber(type);
            destinationAddress.setNumberingPlan(plan);
            destinationAddress.setValue(lenght, buff);
        };
        inline void setDestinationAddress(Address& address) 
        { // Copies address value from 'address' to static structure 
            destinationAddress = address;     
        };
        inline Address& getDestinationAddress() {
            return destinationAddress; 
        };
       
        inline void setWaitTime(time_t time) {
            waitTime = time;
        };
        inline time_t getWaitTime() {
            return waitTime;
        };
        
        inline void setValidTime(time_t time) {
            validTime = time;
        };
        inline time_t getValidTime() {
            return validTime;
        };
        
        inline void setSubmitTime(time_t time) {
            submitTime = time;
        };
        inline time_t getSubmitTime() {
            return submitTime;
        };
        
        inline void setDeliveryTime(time_t time) {
            deliveryTime = time;
        };
        inline time_t getDeliveryTime() {
            return deliveryTime;
        };
        
        inline void setMessageReference(uint8_t mr) {
            messageReference = mr;
        };
        inline uint8_t getMessageReference() {
            return messageReference;
        };
       
        inline void setMessageIdentifier(uint8_t mi) {
            messageIdentifier = mi;
        };
        inline uint8_t getMessageIdentifier() {
            return messageIdentifier;
        };
       
        inline void setPriority(uint8_t pri) {
            priority = pri;
        };
        inline uint8_t getPriority() {
            return priority;
        };
       
        inline void setProtocolIdentifier(uint8_t pid) {
            protocolIdentifier = pid;
        };
        inline uint8_t getProtocolIdentifier() {
            return protocolIdentifier;
        };
       
        inline void setStatusReportRequested(bool req) {
            statusReportRequested = req;
        };
        inline bool isStatusReportRequested() {
            return statusReportRequested;
        };
       
        inline void setRejectDuplicates(bool rej) {
            rejectDuplicates = rej;
        };
        inline bool isRejectDuplicates() {
            return rejectDuplicates;
        };
       
        inline void setFailureCause(uint8_t cause) {
            failureCause = cause;
        };
        inline uint8_t getFailureCause() {
            return failureCause;
        };
        
        inline void setMessageBody(uint8_t lenght, uint8_t scheme, 
                                         bool header, uint8_t* buff) 
        { // Copies body data from 'buff' to static structure 
            messageBody.setCodingScheme(scheme);
            messageBody.setHeaderIndicator(header);
            messageBody.setData(lenght, buff);
        };
        inline void setMessageBody(Body& body) 
        { // Copies body data from 'body' to static structure 
            messageBody = body;     
        };
        inline Body& getMessageBody() {
            return messageBody; 
        };
    };

}}

#endif


