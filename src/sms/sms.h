#ifndef SMS_DECLARATIONS
#define SMS_DECLARATIONS

/**
 * ���� �������� �������� ���������� ��������� ������ ��� ������������� SMS
 * � ������� SMS ������. ������������ �������� ��������.
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see MessageStore
 */
 
#include <inttypes.h>
#include <time.h>
#include <string.h>

#include <util/debug.h>

namespace smsc { namespace sms
{
    const int MAX_ADDRESS_VALUE_LENGTH = 21;
    const int MAX_SHORT_MESSAGE_LENGTH = 200;
    
    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef uint8_t     SMSData[MAX_SHORT_MESSAGE_LENGTH];
    typedef uint32_t    SMSId;
    
    /**
     * ��������� Address ������������� ��� �������� 
     * ������� � SMS ���������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see SMS
     */
    struct Address
    {
        uint8_t      lenght, type, plan;
        AddressValue value;
        
        /**
         * Default �����������, ������ �������������� ��������� ���� ������
         */
        Address() : lenght(0), type(0), plan(0) 
        {
            value[0] = '0'; value[1] = '\0';
        };
        
        /**
         * ����������� ��� Address, �������������� ���� ��������� ��������� �������.
         * �������� ������ �� ������ � ����
         * 
         * @param _len   ������ ������ _value
         * @param _type  ��� ������
         * @param _plan  ���� ���������
         * @param _value �������� ������
         */
        Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
            : lenght(_len), type(_type), plan(_plan) 
        { 
            setValue(_len, _value);
        };
  
        /**
         * ����������� �����������, ������������ ��� �������� ������ �� �������
         * 
         * @param addr   ������� ������.
         */
        Address(const Address& addr) 
            : lenght(addr.lenght), type(addr.type), plan(addr.plan) 
        {
            setValue(addr.lenght, addr.value);   
        };

        /**
         * ��������������� �������� '=',
         * ������������ ��� ����������� ������� ���� � �����
         * 
         * @param addr   ������ ����� ��������� '='
         * @return ������ �� ����
         */
        Address& operator =(const Address& addr) 
        {
            type = addr.type; plan = addr.plan; 
            setValue(addr.lenght, addr.value);
            return (*this);
        };
       
        /**
         * ����� ������������� �������� ������ � ��� ������.
         * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   ������ ������ ������
         * @param _value �������� ������ ������
         */
        inline void setValue(uint8_t _len, const char* _value) 
        {
            __require__( _len<sizeof(AddressValue) );
            
            if (_len && _value)
            {
                memcpy(value, _value, _len*sizeof(uint8_t));
                value[lenght = _len] = '\0';
            }
            else 
            {
                memset(value, 0, sizeof(AddressValue));
                value[0] = '0';
                lenght = 0;
            }
        };
        
        /**
         * ����� �������� �������� ������ � ���������� ��� ������
         * 
         * @param _value ��������� �� ����� ���� ����� ������������ �������� ������
         *               ����� ������ ����� ������ �� ������
         *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
         * @return ������ ������
         */
        inline uint8_t getValue(char* _value) const 
        {
            __require__(value);
            
            if (lenght)
            {
                memcpy(_value, value, lenght*sizeof(uint8_t));
                _value[lenght] = '\0';
            }
            return lenght;
        }
        
        /**
         * ���������� ������ ������
         * 
         * @return ������ ������
         */
        inline uint8_t getLenght() const 
        {
            return lenght;
        };
       
        /**
         * ������������� ��� ������
         * 
         * @param _type  ��� ������
         */
        inline void setTypeOfNumber(uint8_t _type) 
        {
            type = _type;
        };
        
        /**
         * ���������� ��� ������
         * 
         * @param _type  ��� ������
         */
        inline uint8_t getTypeOfNumber() const 
        { 
            return type; 
        };
      
        /**
         * ������������� ���� ��������� ������
         * 
         * @param _plan  ���� ��������� ������
         */
        inline void setNumberingPlan(uint8_t _plan) 
        {
            plan = _plan;
        };
        
        /**
         * ���������� ���� ��������� ������
         * 
         * @return ���� ��������� ������
         */
        inline uint8_t getNumberingPlan() const 
        {
            return plan;
        };
    };

    /**
     * ��������� ��������� ���� ���������
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see SMS
     */
    struct Body
    {
        bool        header;
        uint8_t     scheme;
        uint8_t     lenght;
        SMSData     data;

        /**
         * Default �����������, ������ �������������� ��������� ���� ������
         */
        Body() : header(false), lenght(0), scheme(0) 
        {
            //memset((void *)data, 0, sizeof(data));
        };
        
        /**
         * ����������� ��� Body, �������������� ���� ��������� ��������� �������.
         * �������� ������ �� ������ � ����
         * 
         * @param _len    ������ ������ _data
         * @param _scheme ����� ����������� ���� ���������
         * @param _header �������������� �������� �� ���� ���������
         * @param _data   �������������� ���� ���������
         */
        Body(uint8_t _len, uint8_t _scheme, bool _header, const uint8_t* _data)
            : header(_header), lenght(_len), scheme(_scheme) 
        { 
            setData(_len, _data);
        };
        
        /**
         * ����������� �����������, ������������ ��� �������� ���� �� �������
         * 
         * @param body   ������� ����.
         */
        Body(const Body& body) 
            : header(body.header), lenght(body.lenght), scheme(body.scheme)
        {
            setData(body.lenght, body.data);   
        };
        
        /**
         * ��������������� �������� '=',
         * ������������ ��� ����������� ��� ��������� ���� � �����
         * 
         * @param body   ������ ����� ��������� '='
         * @return ������ �� ����
         */
        Body& operator =(const Body& body) 
        {
            header = body.header; scheme = body.scheme;
            setData(body.lenght, body.data);
            return (*this);
        };

        /**
         * ����� ������������� �������� ���� � ��� ������.
         * ������ ���� ������ ���� ������ ���� ����� MAX_SMS_DATA_LENGTH.
         * 
         * @param _len   ������ ������ ����
         * @param _value �������� ������ ����
         */
        inline void setData(uint8_t _len, const uint8_t* _data) 
        {
            __require__( _len<=sizeof(SMSData) );
            
            if (_len && _data)
            {
                memcpy(data, _data, _len*sizeof(uint8_t));
                lenght = _len;
                if (lenght < sizeof(SMSData)) 
                {
                    data[lenght] = '\0';
                }
            }
            else
            {
                memset(data, 0, sizeof(SMSData));
                lenght = _len;
            }
        };
        
        /**
         * ����� �������� �������� ���� � ���������� ��� ������
         * 
         * @param _data  ��������� �� ����� ���� ����� ������������ �������� ����
         *               ����� ������ ����� ������ �� ������
         *               MAX_SMS_DATA_LENGTH, ����� ������� ����� ��������
         * @return ������ ����
         */
        inline uint8_t getData(uint8_t* _data) const 
        {
            __require__(data);

            if (lenght)
            {
                memcpy(_data, data, lenght*sizeof(uint8_t));
            }
            return lenght;
        };
        
        /**
         * ������������� ����� ����������� ���� ���������
         * 
         * @param _scheme ����� ����������� ���� ���������
         */
        inline void setCodingScheme(uint8_t _scheme) 
        {
            scheme = _scheme;
        };
        
        /**
         * ���������� ����� ����������� ���� ���������
         * 
         * @return ����� ����������� ���� ���������
         */
        inline uint8_t getCodingScheme() const 
        {
            return scheme;
        };
        
        /**
         * ������������� ������� ������� ��������� � ���� ���������
         * 
         * @param _header ������� ������� ��������� � ���� ���������
         */
        inline void setHeaderIndicator(bool _header) 
        {
            header = _header;
        };
        
        /**
         * ��������� ���������� �� ������� ������� ��������� � ���� ���������
         * 
         * @return �� / ���
         */
        inline bool isHeaderIndicator() const 
        {
            return header;
        };

        /**
         * ����� ���������� ���� ���������. 
         * �������� ������, ����� ������������� � ���������� ����������.
         * ���� �� �����������������
         * 
         * @return �������������� ���� ���������. ������ NULL.
         */
        char* getDecodedText();
    };
   
    /**
     * ��������� ��������� SMS � ��������� SMS ������
     * 
     * @see SMS
     */
    typedef enum { 
        ENROUTE=0, DELIVERED=1, 
        EXPIRED=2, UNDELIVERABLE=3,
        DELETED=4 
    } State;
   
    /**
     * ��������� ����������� SMS � ��������� ��������
     * SMS �����. ������������ �������� ��������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see MessageStore
     */
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

        
        /**
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         */
        SMS() : state(ENROUTE) {}; 
        
        /**
         * ����������� �����������, ������������ ��� ��������
         * SMS �� �������
         * 
         * @param sms    ������� SMS
         */
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
        
        /**
         * ��������������� �������� '=',
         * ������������ ��� ����������� ���������
         * 
         * @param sms   ������ ����� ��������� '='
         * @return ������ �� ����
         */
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
        
        /**
         * ������������� ��������� ���������
         * 
         * @param state  ����� ���������
         */
        inline void setState(State state) 
        {
            this->state = state;
        };
        
        /**
         * ���������� ��������� ���������
         * 
         * @return ��������� ���������
         */
        inline State getState() const 
        {
            return state;
        };

        /**
         * ������������� ����� �����������.
         * �������� ����� �� ���������� ���������
         * 
         * @param lenght ������ ������ (0 < lenght <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   ��� ������
         * @param plan   ���� ��������� ������
         * @param buff   �������� ������
         * @see Address
         */
        inline void setOriginatingAddress(uint8_t lenght, uint8_t type, 
                                          uint8_t plan, const char* buff) 
        { // Copies address value from 'buff' to static structure
            originatingAddress.setTypeOfNumber(type);
            originatingAddress.setNumberingPlan(plan);
            originatingAddress.setValue(lenght, buff);
        };
        
        /**
         * ������������� ����� �����������
         * �������� ����� �� ���������� ���������
         * 
         * @param address ����� ����� �����������
         * @see Address
         */
        inline void setOriginatingAddress(const Address& address) 
        { // Copies address value from 'address' to static structure 
            originatingAddress = address;     
        };
        
        /**
         * ���������� ����� �����������
         * 
         * @return ����� �����������
         * @see Address
         */
        inline const Address& getOriginatingAddress() const 
        {
            return originatingAddress; 
        };

        /**
         * ������������� ����� ����������
         * �������� ����� �� ���������� ���������
         * 
         * @param lenght ������ ������ (0 < lenght <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   ��� ������
         * @param plan   ���� ��������� ������
         * @param buff   �������� ������
         * @see Address
         */
        inline void setDestinationAddress(uint8_t lenght, uint8_t type, 
                                          uint8_t plan, const char* buff) 
        { // Copies address value from 'buff' to static structure 
            __require__(lenght);

            destinationAddress.setTypeOfNumber(type);
            destinationAddress.setNumberingPlan(plan);
            destinationAddress.setValue(lenght, buff);
        };
        
        /**
         * ������������� ����� ����������
         * �������� ����� �� ���������� ���������
         * 
         * @param address ����� ����� ����������
         * @see Address
         */
        inline void setDestinationAddress(const Address& address) 
        { // Copies address value from 'address' to static structure 
            destinationAddress = address;     
        };
        
        /**
         * ���������� ����� ����������
         * 
         * @return ����� ����������
         * @see Address
         */
        inline const Address& getDestinationAddress() const
        {
            return destinationAddress; 
        };
       
        /**
         * ������������� ����� ��������. 
         * 
         * @param time   ����, ����� ��������� ������ ���� ����������
         *               (�� �������� ��������).
         */
        inline void setWaitTime(time_t time) 
        {
            waitTime = time;
        };
        
        /**
         * ���������� ����� ��������.
         * 
         * @return ����, ����� ��������� ������ ���� ����������
         *         (�� �������� ��������).
         */
        inline time_t getWaitTime() const 
        {
            return waitTime;
        };
        
        /**
         * ������������� ����� ���������� ���������. 
         * 
         * @param time   ����, �� ������� ��������� �������
         *               (�� �������� �������).
         */
        inline void setValidTime(time_t time) 
        {
            validTime = time;
        };
        
        /**
         * ���������� ����� ���������� ���������.
         * 
         * @return time   ����, �� ������� ��������� �������
         *         (�� �������� �������).
         */
        inline time_t getValidTime() const 
        {
            return validTime;
        };
        
        /**
         * ������������� ����� ����������� ��������� � SMSC
         * 
         * @param time   ����� ����������� ��������� � SMSC
         */
        inline void setSubmitTime(time_t time) 
        {
            submitTime = time;
        };
        
        /**
         * ���������� ����� ����������� ��������� � SMSC
         * 
         * @return ����� ����������� ��������� � SMSC
         */
        inline time_t getSubmitTime() const 
        {
            return submitTime;
        };
        
        /**
         * ������������� ����� ��������� ������� �������� ��������� �� SMSC
         * 
         * @param time   ����� ��������� ������� �������� ��������� �� SMSC
         */
        inline void setDeliveryTime(time_t time) 
        {
            deliveryTime = time;
        };
        
        /**
         * ���������� ����� ��������� ������� �������� ��������� �� SMSC
         * 
         * @return ����� ��������� ������� �������� ��������� �� SMSC
         */
        inline time_t getDeliveryTime() const 
        {
            return deliveryTime;
        };
        
        /**
         * ������������� ����� ��������� (MR), 
         * ��� ������������� ��������� ���������� � ������ ������
         * 
         * @param mr     ����������������� ����� ��������� (MR)
         */
        inline void setMessageReference(uint8_t mr) 
        {
            messageReference = mr;
        };
        
        /**
         * ���������� ����� ��������� (MR),
         * ��� ������������� ��������� ���������� � ������ ������
         * 
         * @return ����������������� ����� ��������� (MR)
         */
        inline uint8_t getMessageReference() const 
        {
            return messageReference;
        };
       
        /**
         * ������������� ����� ��������� (SMI), 
         * ��� ������������� ��������� ��������� �� ���� �����
         * ! ������ ����� �� ����� !
         * 
         * @param mi     ����������������� ����� ��������� (SMI)
         */
        inline void setMessageIdentifier(uint8_t mi) 
        {
            messageIdentifier = mi;
        };
        
        /**
         * ���������� ����� ��������� (SMI),
         * ��� ������������� ��������� ��������� �� ���� �����
         * ! ������ ����� �� ����� !
         * 
         * @return ����������������� ����� ��������� (SMI)
         */
        inline uint8_t getMessageIdentifier() const 
        {
            return messageIdentifier;
        };
       
        /**
         * ������������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pri    ��������� ���������
         */
        inline void setPriority(uint8_t pri) 
        {
            priority = pri;
        };
        
        /**
         * ���������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ��������� ���������
         */
        inline uint8_t getPriority() const 
        {
            return priority;
        };
       
        /**
         * ������������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pid    ������ �������� ���������
         */
        inline void setProtocolIdentifier(uint8_t pid) 
        {
            protocolIdentifier = pid;
        };
        
        /**
         * ���������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ������ �������� ���������
         */
        inline uint8_t getProtocolIdentifier() const 
        {
            return protocolIdentifier;
        };
       
        /**
         * ������������� �������, ����� �� ����� � �������� ��������� 
         * 
         * @param req    �������, ����� �� ����� � �������� ���������
         */
        inline void setStatusReportRequested(bool req) 
        {
            statusReportRequested = req;
        };
        
        /**
         * ���������� �������, ����� �� ����� � �������� ���������
         * 
         * @return �������, ����� �� ����� � �������� ��������� (�� / ���)
         */
        inline bool isStatusReportRequested() const 
        {
            return statusReportRequested;
        };
       
        /**
         * ������������� �������, ����� �� �������� ��������� ���������
         * (� ����������� MR � ������ ������) 
         * 
         * @param rej    �������, ����� �� �������� ��������� ��������� 
         */
        inline void setRejectDuplicates(bool rej) 
        {
            rejectDuplicates = rej;
        };
        
        /**
         * ���������� �������, ����� �� �������� ��������� ���������
         * (� ����������� MR � ������ ������) 
         * 
         * @return �������, ����� �� �������� ��������� ��������� (�� / ���)
         */
        inline bool isRejectDuplicates() const 
        {
            return rejectDuplicates;
        };
       
        /**
         * ������������� ������� � ������ 
         * ������/��������������/���������� ���������
         * 
         * @param cause  ������� ������/��������������/���������� ���������
         */
        inline void setFailureCause(uint8_t cause) 
        {
            failureCause = cause;
        };
        
        /**
         * ���������� ������� � ������
         * ������/��������������/���������� ���������
         * 
         * @return ������� ������/��������������/���������� ���������
         */
        inline uint8_t getFailureCause() const 
        {
            return failureCause;
        };
        
        /**
         * ������������� ���� ���������
         * 
         * @param lenght ������ ���� ���������
         * @param scheme ����� ��������� ���� ���������
         * @param header �������, �������� �� ���� ���������
         * @param buff   �������������� ������ � ���� ��������� 
         * @see Body
         */
        inline void setMessageBody(uint8_t lenght, uint8_t scheme, 
                                   bool header, const uint8_t* buff) 
        { // Copies body data from 'buff' to static structure 
            messageBody.setCodingScheme(scheme);
            messageBody.setHeaderIndicator(header);
            messageBody.setData(lenght, buff);
        };
        
        /**
         * ������������� ���� ���������
         * 
         * @param body ���� ���������
         * @see Body
         */
        inline void setMessageBody(const Body& body) 
        { // Copies body data from 'body' to static structure 
            messageBody = body;     
        };
        
        /**
         * ���������� ���� ���������
         * 
         * @return ���� ���������
         * @see Body
         */
        inline const Body& getMessageBody() const
        {
            return messageBody; 
        };
    };

}}

#endif


