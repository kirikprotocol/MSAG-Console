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
    const int MAX_ESERVICE_TYPE_LENGTH = 5;
    const int MAX_ADDRESS_VALUE_LENGTH = 20;
    //const int MAX_SHORT_MESSAGE_LENGTH = 200; depricated
    const int MAX_BODY_LENGTH          = 2000; 
    // move it to SQL statements processing
    
    //const char* DEFAULT_ETSI_GSM_SEVICE_NAME = "GSM-SM";

    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
    //typedef uint8_t     SMSData[MAX_SHORT_MESSAGE_LENGTH]; depricated
    typedef uint64_t    SMSId;

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
        uint8_t      length, type, plan;
        AddressValue value;
        
        /**
         * Default �����������, ������ �������������� ��������� ���� ������
         */
        Address() : length(1), type(0), plan(0) 
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
            : length(_len), type(_type), plan(_plan) 
        { 
            setValue(_len, _value);
        };
  
        /**
         * ����������� �����������, ������������ ��� �������� ������ �� �������
         * 
         * @param addr   ������� ������.
         */
        Address(const Address& addr) 
            : length(addr.length), type(addr.type), plan(addr.plan) 
        {
            setValue(addr.length, addr.value);   
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
            setValue(addr.length, addr.value);
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
            __require__(_len && _value && _value[0]
                        && _len<sizeof(AddressValue));
            
            memcpy(value, _value, _len*sizeof(uint8_t));
            value[length = _len] = '\0';
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
            __require__(_value);
            
            if (length)
            {
                memcpy(_value, value, length*sizeof(uint8_t));
                _value[length] = '\0';
            }
            return length;
        }
        
        // depricated
        inline uint8_t getLenght() const 
        {
            return getLength();
        };
        
        /**
         * ���������� ������ ������
         * 
         * @return ������ ������
         */
        inline uint8_t getLength() const 
        {
            return length;
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
     * ��������� Descriptor ������������� ��� �������� 
     * ���������� ���������������� ����������������� �������
     * (��������, �������� ��� ������� ��������� SMS)
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see SMS
     */
    struct Descriptor
    {
        uint8_t         mscLength, imsiLength;
        AddressValue    msc, imsi;
        uint32_t        sme;
        
        /**
         * Default �����������, ������ �������������� ��������� ���� ������
         */
        Descriptor() : mscLength(0), imsiLength(0), sme(0) 
        {
            msc[0] = '\0'; imsi[0] = '\0';
        };
        
        /**
         * ����������� ��� Descriptor, �������������� ���� ��������� ��������� �������.
         * �������� ������ �� ������� � ����
         * 
         * @param _mscLen   ������ ������ _msc
         * @param _value �������� ������ MSC
         * @param _imsiLen   ������ ������ _imsi
         * @param _value �������� ������ IMSI
         * @param _sme ����� SME
         */
        Descriptor(uint8_t _mscLen, const char* _msc,
                   uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
            : mscLength(_mscLen), imsiLength(_imsiLen), sme(_sme)
        { 
            setMsc(mscLength, _msc);
            setImsi(imsiLength, _imsi);
        };
  
        /**
         * ����������� �����������, ������������ ���
         * �������� ����������� �� ������� 
         * 
         * @param descr ������� �����������.
         */
        Descriptor(const Descriptor& descr) 
            : mscLength(descr.mscLength), 
                imsiLength(descr.imsiLength), sme(descr.sme)
        {
            setMsc(descr.mscLength, descr.msc);
            setImsi(descr.imsiLength, descr.imsi);
        };

        /**
         * ��������������� �������� '=',
         * ������������ ��� ����������� ������������ ���� � �����
         * 
         * @param descr   ������ ����� ��������� '='
         * @return ������ �� ����
         */
        Descriptor& operator =(const Descriptor& descr) 
        {
            sme = descr.sme;
            setMsc(descr.mscLength, descr.msc);
            setImsi(descr.imsiLength, descr.imsi);
            return (*this);
        };
       
        /**
         * ����� ������������� �������� ������ MSC � ��� ������.
         * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   ������ ������ ������ MSC
         * @param _value �������� ������ ������ MSC
         */
        inline void setMsc(uint8_t _len, const char* _value) 
        {
            __require__( _len<sizeof(AddressValue) );
            
            if (_len && _value)
            {
                memcpy(msc, _value, _len*sizeof(uint8_t));
                msc[mscLength = _len] = '\0';
            }
            else 
            {
                memset(msc, 0, sizeof(AddressValue));
                mscLength = 0;
            }
        };
        
        /**
         * ����� ������������� �������� ������ IMSI � ��� ������.
         * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   ������ ������ ������ IMSI
         * @param _value �������� ������ ������ IMSI
         */
        inline void setImsi(uint8_t _len, const char* _value) 
        {
            __require__( _len<sizeof(AddressValue) );
            
            if (_len && _value)
            {
                memcpy(imsi, _value, _len*sizeof(uint8_t));
                imsi[imsiLength = _len] = '\0';
            }
            else 
            {
                memset(imsi, 0, sizeof(AddressValue));
                imsiLength = 0;
            }
        };
        
        /**
         * ����� �������� �������� ������ MSC � ���������� ��� ������
         * 
         * @param _value ��������� �� ����� ���� ����� ������������ �������� 
         *               ������ MSC. ����� ������ ����� ������ �� ������
         *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
         * @return ������ ������ MSC
         */
        inline uint8_t getMsc(char* _value) const 
        {
            __require__(_value);
            
            if (mscLength)
            {
                memcpy(_value, msc, mscLength*sizeof(uint8_t));
                _value[mscLength] = '\0';
            }
            return mscLength;
        }
        
        /**
         * ����� �������� �������� ������ IMSI � ���������� ��� ������
         * 
         * @param _value ��������� �� ����� ���� ����� ������������ �������� 
         *               ������ IMSI. ����� ������ ����� ������ �� ������
         *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
         * @return ������ ������ IMSI
         */
        inline uint8_t getImsi(char* _value) const 
        {
            __require__(_value);
            
            if (imsiLength)
            {
                memcpy(_value, imsi, imsiLength*sizeof(uint8_t));
                _value[imsiLength] = '\0';
            }
            return imsiLength;
        }
        
        // depricated
        inline uint8_t getMscLenght() const 
        {
            return getMscLength();
        };
        
        /**
         * ���������� ������ ������ MSC
         * 
         * @return ������ ������ MSC
         */
        inline uint8_t getMscLength() const 
        {
            return mscLength;
        };
        
        // depricated
        inline uint8_t getImsiLenght() const 
        {
            return getImsiLength();
        };
        
        /**
         * ���������� ������ ������ IMSI
         * 
         * @return ������ ������ IMSI
         */
        inline uint8_t getImsiLength() const 
        {
            return imsiLength;
        };
        
        /**
         * ������������� ����� SME
         * 
         * @return ������ ������ IMSI
         */
        inline void setSmeNumber(uint32_t _sme)
        {
            sme = _sme;
        };
        
        /**
         * ���������� ����� SME
         * 
         * @return ����� SME
         */
        inline uint32_t getSmeNumber() const 
        {
            return sme;
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
    private:

        uint8_t*    buff;
        int         buffLen;

    public:    
        
        /**
         * Default �����������, ������ �������������� ���� ������
         */
        Body() : buff(0), buffLen(0) {};
        
        /**
         * ����������� ��� Body, �������������� ���� ��������� ��������� �������.
         * �������� ������ �� ������ � ����
         * 
         * @param data   ������ � ���� ���������
         * @param len    ������ ������ data
         */
        Body(uint8_t* data, int len)
            : buff(0), buffLen(0)
        { 
            setBuffer(data, len);
        };

        /**
         * ����������� �����������, ������������ ��� �������� ���� �� �������
         * 
         * @param body   ������� ����.
         */
        Body(const Body& body) 
            : buff(0), buffLen(0)
        {
            setBuffer(body.getBuffer(), body.getBufferLength());
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
            setBuffer(body.getBuffer(), body.getBufferLength());
            return (*this);
        };

        /**
         * ����� ���������� ���� ���������. 
         * �������� ������, ����� ������������� � ���������� ����������.
         * ���� �� �����������������
         * 
         * @return �������������� ���� ���������. ������ NULL.
         */
        char* getDecodedText() 
        {
            return 0L;
        };
        
        uint8_t* getBuffer() const
        {
            return buff;
        };
        
        int getBufferLength() const
        {
            return buffLen;
        };
        
        void setBuffer(uint8_t* buffer,int length)
        {
            if (buff) 
            {
                buff = 0; buffLen = 0;
                delete buff;
            }
            
            if (buffer && length>0)
            {
                buff = new uint8_t[buffLen = length];
                memcpy(buff, buffer, length);
            }
        };
    };
   
    const uint8_t SMSC_BYTE_ENROUTE_STATE       = (uint8_t)0;
    const uint8_t SMSC_BYTE_DELIVERED_STATE     = (uint8_t)1;
    const uint8_t SMSC_BYTE_EXPIRED_STATE       = (uint8_t)2;
    const uint8_t SMSC_BYTE_UNDELIVERABLE_STATE = (uint8_t)3;
    const uint8_t SMSC_BYTE_DELETED_STATE       = (uint8_t)4;

    /**
     * ��������� ��������� SMS � ��������� SMS ������
     * 
     * @see SMS
     */
    typedef enum { 
        ENROUTE         = SMSC_BYTE_ENROUTE_STATE, 
        DELIVERED       = SMSC_BYTE_DELIVERED_STATE, 
        EXPIRED         = SMSC_BYTE_EXPIRED_STATE, 
        UNDELIVERABLE   = SMSC_BYTE_UNDELIVERABLE_STATE, 
        DELETED         = SMSC_BYTE_DELETED_STATE 
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
        /*                      !!! Depricated !!!
        
        State       state;
        uint16_t    messageReference;
        
        Address     originatingAddress;
        Address     destinationAddress;
        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;
        
        time_t      waitTime;       // �����/���� � �������� �������� ���������
        time_t      validTime;      // �����/���� �� �������� ��������� �������
        time_t      submitTime;     // �����/���� ����������� �� SMSC
        time_t      lastTime;       // �����/���� ��������� ������� ��������
        time_t      nextTime;       // �����/���� ��������� ������� ��������
        
        uint8_t     priority;
        uint8_t     protocolIdentifier;
        
        uint8_t     deliveryReport;
        bool        needArchivate;
        
        uint8_t     failureCause;   // ������� ������� ��������� �������
        uint32_t    attempts;       // ���������� ���������� ������� ��������
        
        Body        messageBody;    // �������������� & ������ ���� ���������
        EService    eServiceType;   

        SMSId       receiptSmsId;   // id ��������� �� ������� ��� �����-������
        uint8_t     esmClass;       // ��� ���������: ����������, ����� ... 
        */

        State       state;
        time_t      submitTime;     // �����/���� ����������� �� SMSC
        time_t      validTime;      // �����/���� �� �������� ��������� �������
        
        uint32_t    attempts;       // ���������� ���������� ������� ��������
        uint8_t     lastResult;     // ��������� ��������� �������
        time_t      lastTime;       // �����/���� ��������� ������� ��������
        time_t      nextTime;       // �����/���� ��������� ������� ��������
        
        Address     originatingAddress;
        Address     destinationAddress;

        uint16_t    messageReference;
        EService    eServiceType;   

        bool        needArchivate;
        uint8_t     deliveryReport;
        uint8_t     billingRecord;
        
        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;
        
        Body        messageBody;    // ���� ��������� + PDU ����.
        bool        attach;

        /**
         *                      !!! Depricated !!!                  
         *
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         *
        SMS() : state(ENROUTE), lastTime(0), nextTime(0),
                failureCause(0), attempts(0), receiptSmsId(0), esmClass(0)
        {
            eServiceType[0]='\0';
        };*/

        /**
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         * � ������ ���� ���������� ����������
         */
        SMS() : state(ENROUTE), submitTime(0), validTime(0),
                attempts(0), lastResult(0), lastTime(0), nextTime(0),
                messageReference(0), needArchivate(true),
                deliveryReport(0), billingRecord(0), attach(false)
        {
            eServiceType[0]='\0';
        }; 

        
        /**
         *                      !!! Depricated !!!                  
         *
         * ����������� �����������, ������������ ��� ��������
         * SMS �� �������
         * 
         * @param sms    ������� SMS
         *
        SMS(const SMS& sms) :
            state(sms.state), 
            messageReference(sms.messageReference),
            originatingAddress(sms.originatingAddress),
            destinationAddress(sms.destinationAddress), 
            originatingDescriptor(sms.originatingDescriptor),
            destinationDescriptor(sms.destinationDescriptor), 
            waitTime(sms.waitTime), validTime(sms.validTime), 
            submitTime(sms.submitTime), lastTime(sms.lastTime),
            nextTime(sms.nextTime), priority(sms.priority),
            protocolIdentifier(sms.protocolIdentifier),
            deliveryReport(sms.deliveryReport),
            needArchivate(sms.needArchivate),
            failureCause(sms.failureCause),
            attempts(sms.attempts),
            messageBody(sms.messageBody),
            receiptSmsId(sms.receiptSmsId),
            esmClass(sms.esmClass)
        {
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
        };*/
        
        /**
         * ����������� �����������, ������������ ��� ��������
         * SMS �� �������
         * 
         * @param sms    ������� SMS
         */
        SMS(const SMS& sms) :
            state(sms.state),
            submitTime(sms.submitTime), 
            validTime(sms.validTime),
            attempts(sms.attempts),
            lastResult(sms.lastResult),
            lastTime(sms.lastTime),
            nextTime(sms.nextTime),
            originatingAddress(sms.originatingAddress),
            destinationAddress(sms.destinationAddress), 
            messageReference(sms.messageReference),
            needArchivate(sms.needArchivate),
            deliveryReport(sms.deliveryReport),
            billingRecord(sms.billingRecord),
            originatingDescriptor(sms.originatingDescriptor),
            destinationDescriptor(sms.destinationDescriptor), 
            messageBody(sms.messageBody),
            attach(sms.attach)
        {
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
        };

        
        /**
         *                      !!! Depricated !!!                  
         *
         * ��������������� �������� '=',
         * ������������ ��� ����������� ���������
         * 
         * @param sms   ������ ����� ��������� '='
         * @return ������ �� ����
         *
        SMS& operator =(const SMS& sms) 
        {
            state = sms.state; 
            originatingAddress = sms.originatingAddress;
            destinationAddress = sms.destinationAddress;
            originatingDescriptor = sms.originatingDescriptor;
            destinationDescriptor = sms.destinationDescriptor;
            waitTime = sms.waitTime; validTime = sms.validTime; 
            submitTime = sms.submitTime; lastTime = sms.lastTime;
            nextTime = sms.nextTime; priority = sms.priority; 
            messageReference = sms.messageReference; 
            protocolIdentifier = sms.protocolIdentifier;
            deliveryReport = sms.deliveryReport;
            needArchivate = sms.needArchivate;
            failureCause = sms.failureCause;
            attempts = sms.attempts; 
            messageBody = sms.messageBody;
            receiptSmsId = sms.receiptSmsId;
            esmClass = sms.esmClass;
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
            return (*this);
        };*/

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
            submitTime = sms.submitTime;
            validTime = sms.validTime;
            attempts = sms.attempts;
            lastResult = sms.lastResult;
            lastTime = sms.lastTime;
            nextTime = sms.nextTime;
            originatingAddress = sms.originatingAddress;
            destinationAddress = sms.destinationAddress;
            messageReference = sms.messageReference;
            needArchivate = sms.needArchivate;
            deliveryReport = sms.deliveryReport;
            billingRecord = sms.billingRecord;
            originatingDescriptor = sms.originatingDescriptor;
            destinationDescriptor = sms.destinationDescriptor;
            messageBody = sms.messageBody;
            attach = sms.attach;
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
            return (*this);
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
         * @param length ������ ������ (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   ��� ������
         * @param plan   ���� ��������� ������
         * @param buff   �������� ������
         * @see Address
         */
        inline void setOriginatingAddress(uint8_t length, uint8_t type, 
                                          uint8_t plan, const char* buff) 
        { // Copies address value from 'buff' to static structure
            originatingAddress.setTypeOfNumber(type);
            originatingAddress.setNumberingPlan(plan);
            originatingAddress.setValue(length, buff);
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
         * ���������� ����� �����������
         * 
         * @return ����� �����������
         * @see Address
         */
        inline Address& getOriginatingAddress()
        {
            return originatingAddress; 
        };

        /**
         * ������������� ����� ����������
         * �������� ����� �� ���������� ���������
         * 
         * @param length ������ ������ (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   ��� ������
         * @param plan   ���� ��������� ������
         * @param buff   �������� ������
         * @see Address
         */
        inline void setDestinationAddress(uint8_t length, uint8_t type, 
                                          uint8_t plan, const char* buff) 
        { // Copies address value from 'buff' to static structure 
            destinationAddress.setTypeOfNumber(type);
            destinationAddress.setNumberingPlan(plan);
            destinationAddress.setValue(length, buff);
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
         * ���������� ����� ����������
         * 
         * @return ����� ����������
         * @see Address
         */
        inline Address& getDestinationAddress()
        {
            return destinationAddress; 
        };
       
        /**
         * ������������� ���������� �����������
         * �������� ����� �� ���������� ���������
         * 
         * @param descriptor ����� ���������� �����������
         * @see Descriptor
         */
        inline void setOriginatingDescriptor(const Descriptor& descriptor) 
        { // Copies descriptor from 'descriptor' to static structure 
            originatingDescriptor = descriptor;     
        };
        
        /**
         * ������������� ���������� �����������
         * �������� ����� �� ���������� ���������
         *
         * @param _mscLen   ������ ������ _msc
         * @param _value �������� ������ MSC
         * @param _imsiLen   ������ ������ _imsi
         * @param _value �������� ������ IMSI
         * @param _sme ����� SME
         * 
         * @see Descriptor
         */
        inline void setOriginatingDescriptor(uint8_t _mscLen, const char* _msc,
                   uint8_t _imsiLen, const char* _imsi, uint32_t _sme) 
        { // Copies descriptor from 'descriptor' to static structure 
            originatingDescriptor.setMsc(_mscLen, _msc);
            originatingDescriptor.setImsi(_imsiLen, _imsi);
            originatingDescriptor.setSmeNumber(_sme);
        };
        
        /**
         * ���������� ���������� �����������
         * 
         * @return ���������� �����������
         * @see Descriptor
         */
        inline const Descriptor& getOriginatingDescriptor() const 
        {
            return originatingDescriptor; 
        };
        
        /**
         * ���������� ���������� �����������
         * 
         * @return ���������� �����������
         * @see Descriptor
         */
        inline Descriptor& getOriginatingDescriptor()
        {
            return originatingDescriptor; 
        };
        
        /**
         * ������������� ���������� ����������
         * �������� ����� �� ���������� ���������
         * 
         * @param descriptor ����� ���������� ����������
         * @see Descriptor
         *
        inline void setDestinationDescriptor(const Descriptor& descriptor) 
        { // Copies descriptor from 'descriptor' to static structure 
            destinationDescriptor = descriptor;     
        };*/
        
        /**
         * ���������� ���������� ����������
         * 
         * @return ���������� ����������
         * @see Descriptor
         */
        inline const Descriptor& getDestinationDescriptor() const 
        {
            return destinationDescriptor; 
        };
        
        /**
         * ���������� ���������� ����������
         * 
         * @return ���������� ����������
         * @see Descriptor
         *
        inline Descriptor& getDestinationDescriptor()
        {
            return destinationDescriptor; 
        };*/
        

        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ����� ��������. 
         * 
         * @param time   ����, ����� ��������� ������ ���� ����������
         *               (�� �������� ��������).
         */
        inline void setWaitTime(time_t time) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //waitTime = time;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ����� ��������.
         * 
         * @return ����, ����� ��������� ������ ���� ����������
         *         (�� �������� ��������).
         */
        inline time_t getWaitTime() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return waitTime;
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
         *
        inline void setLastTime(time_t time) 
        {
            lastTime = time;
        };*/
        
        /**
         * ���������� ����� ��������� ������� �������� ��������� �� SMSC
         * 
         * @return ����� ��������� ������� �������� ��������� �� SMSC
         */
        inline time_t getLastTime() const 
        {
            return lastTime;
        };
        
        /**
         * ������������� ����� ��������� ������� �������� ��������� �� SMSC
         * 
         * @param time   ����� ��������� ������� �������� ��������� �� SMSC
         */
        inline void setNextTime(time_t time) 
        {
            nextTime = time;
        };
        
        /**
         * ���������� ����� ��������� ������� �������� ��������� �� SMSC
         * 
         * @return ����� ��������� ������� �������� ��������� �� SMSC
         */
        inline time_t getNextTime() const 
        {
            return nextTime;
        };
        
        /**
         * ������������� ����� ��������� (MR), 
         * ��� ������������� ��������� ���������� � ������ ������
         * 
         * @param mr     ����������������� ����� ��������� (MR)
         */
        inline void setMessageReference(uint16_t mr) 
        {
            messageReference = mr;
        };
        
        /**
         * ���������� ����� ��������� (MR),
         * ��� ������������� ��������� ���������� � ������ ������
         * 
         * @return ����������������� ����� ��������� (MR)
         */
        inline uint16_t getMessageReference() const 
        {
            return messageReference;
        };
       
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pri    ��������� ���������
         */
        inline void setPriority(uint8_t pri) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //priority = pri;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ��������� ���������
         */
        inline uint8_t getPriority() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return priority;
        };
       
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pid    ������ �������� ���������
         */
        inline void setProtocolIdentifier(uint8_t pid) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //protocolIdentifier = pid;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ������ �������� ���������
         */
        inline uint8_t getProtocolIdentifier() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return protocolIdentifier;
        };
       
        /**
         * ������������� ��� ������ � �������� ��������� 
         * 
         * @param req    ��� ������ � �������� ���������
         */
        inline void setDeliveryReport(uint8_t report) 
        {
            deliveryReport = report;
        };
        
        /**
         * ���������� ��� ������ � �������� ���������
         * 
         * @return      ��� ������ � �������� ���������
         */
        inline uint8_t getDeliveryReport() const 
        {
            return deliveryReport;
        };
        
        /**
         * ������������� ��� �������� ��������� 
         * 
         * @param req    ��� �������� ���������
         */
        inline void setBillingRecord(uint8_t billing) 
        {
            billingRecord = billing;
        };
        
        /**
         * ���������� ��� �������� ���������
         * 
         * @return      ��� �������� ���������
         */
        inline uint8_t getBillingRecord() const 
        {
            return billingRecord;
        };
       
        /**
         * ������������� �������, ����� �� ������������ ��������� ����� 
         * �������� ��� ���������� ������� �������� 
         * 
         * @param arc    �������, ����� �� ������������ ��������� 
         */
        inline void setArchivationRequested(bool arc) 
        {
            needArchivate = arc;
        };
        
        /**
         * ���������� �������, ����� �� ������������ ��������� ����� 
         * �������� ��� ���������� ������� �������� 
         * 
         * @return �������, ����� �� ����� �� ������������ ��������� (�� / ���)
         */
        inline bool isArchivationRequested() const 
        {
            return needArchivate;
        };
       
        /**
         * ������������� ������� � ������ 
         * ������/��������������/���������� ���������
         * 
         * @param cause  ������� ������/��������������/���������� ���������
         *
        inline void setFailureCause(uint8_t cause) 
        {
            failureCause = cause;
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ������� � ������
         * ������/��������������/���������� ���������
         * 
         * @return ������� ������/��������������/���������� ���������
         */
        inline uint8_t getFailureCause() const 
        {
            __trace__("Method is depricated !!!");
            //return failureCause;
            return getLastResult();
        };
        
        /**
         * ���������� ������� � ������
         * ������/��������������/���������� ��������� ������� �������� ���������
         * 
         * @return ������� ������/��������������/���������� ���������
         */
        inline uint8_t getLastResult() const 
        {
            return lastResult;
        };
        
        /**
         * ������������� ���������� ������ ��� �������� ���������
         * 
         * @param count  ���������� ������ ��� �������� ���������
         *
        inline void setAttemptsCount(uint32_t count) 
        {
            attempts = count;
        };*/
        
        /**
         * ����������� ���������� ������ ��� �������� ��������� 
         * �� ��������� �������� (�� ��������� 1)
         * 
         * @param count  ���������� ������ ��� �������� ���������
         *
        inline void incrementAttemptsCount(uint32_t count=1) 
        {
            attempts += count;
        };*/
        
        /**
         * ���������� ���������� ������ ��� �������� ��������� 
         * 
         * @return ���������� ������ ��� �������� ��������� 
         */
        inline uint32_t getAttemptsCount() const 
        {
            return attempts;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ���� ���������
         * 
         * @param length ������ ���� ���������
         * @param scheme ����� ��������� ���� ���������
         * @param header �������, �������� �� ���� ���������
         * @param buff   �������������� ������ � ���� ��������� 
         * @see Body
         */
        inline void setMessageBody(uint8_t length, uint8_t scheme, 
                                   bool header, const uint8_t* buff) 
        { // Copies body data from 'buff' to static structure 
            /*messageBody.setCodingScheme(scheme);
            messageBody.setHeaderIndicator(header);
            messageBody.setData(length, buff);*/
            __trace__("Method is depricated !!!");
            setMessageBody((uint8_t *)buff, length); 
        };
        
        /**
         * ������������� ���� ���������
         * 
         * @param buff   �������������� ������ � ���� ��������� 
         * @param length ������ ���� ���������
         *
         * @see Body
         */
        inline void setMessageBody(uint8_t* buff, int length) 
        { // Copies body data from 'buff' to static structure 
            messageBody.setBuffer(buff, length);
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
        
        /**
         * ���������� ���� ���������
         * 
         * @return ���� ���������
         * @see Body
         */
        inline Body& getMessageBody()
        {
            return messageBody; 
        };
        
        /**
         * ����� ������������� ���-��� ������� SME.
         * 
         * @param _len   ������ ������ �����-����
         * @param _name ���-��� SME
         */
        inline void setEServiceType(const char* _name) 
        {
            if (_name)
            {
                strncpy(eServiceType, _name, MAX_ESERVICE_TYPE_LENGTH);
                eServiceType[MAX_ESERVICE_TYPE_LENGTH]='\0';
            }
            else
            {
                eServiceType[0]='\0';
            }
            
        };
        
        /**
         * ����� �������� ���-��� ������� SME � ���������� ��� ������
         * 
         * @param _name ��������� �� ����� ���� ����� ������������ ���
         *              ����� ������ ����� ������ �� ������
         *              MAX_ESERVICE_TYPE_LENGTH+1, ����� ������� 
         *              ����� ��������
         */
        inline void getEServiceType(char* _name) const 
        {
            __require__(_name);
            
            if (eServiceType)
            {
                strncpy(_name, eServiceType, MAX_ESERVICE_TYPE_LENGTH);
                _name[MAX_ESERVICE_TYPE_LENGTH]='\0';
            }
            else
            {
                _name[0]='\0';
            }
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� id ���������, �� ������� ��� �����-������.
         * ������������ � ������ ���� esmClass ��� �����-������.
         * 
         * @param id     ����������������� ����� ���������
         */
        inline void setReceiptSmsId(SMSId id) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //receiptSmsId = id;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� id ���������, �� ������� ��� �����-������.
         * ������������ � ������ ���� esmClass ��� �����-������.
         * 
         * @return ����������������� ����� ��������� (MR)
         */
        inline SMSId getReceiptSmsId() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return receiptSmsId;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ���-���������.
         * 
         * @param type      ���-���������    
         */
        inline void setEsmClass(uint8_t type) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //esmClass = type;
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ���-���������.
         * 
         * @return ���-���������
         */
        inline uint8_t getEsmClass() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return esmClass;
        };
    
    };

}}

#endif


