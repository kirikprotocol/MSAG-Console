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
    const int MAX_ESERVICE_TYPE_LENGTH = 6;
    const int MAX_ADDRESS_VALUE_LENGTH = 21;
    const int MAX_SHORT_MESSAGE_LENGTH = 200;
    
    //const char* DEFAULT_ETSI_GSM_SEVICE_NAME = "GSM-SM";

    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
    typedef uint8_t     SMSData[MAX_SHORT_MESSAGE_LENGTH];
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
            __require__(_value);
            
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
        uint8_t         mscLenght, imsiLenght;
        AddressValue    msc, imsi;
        uint32_t        sme;
        
        /**
         * Default �����������, ������ �������������� ��������� ���� ������
         */
        Descriptor() : mscLenght(0), imsiLenght(0), sme(0) 
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
            : mscLenght(_mscLen), imsiLenght(_imsiLen), sme(_sme)
        { 
            setMsc(mscLenght, _msc);
            setImsi(imsiLenght, _imsi);
        };
  
        /**
         * ����������� �����������, ������������ ���
         * �������� ����������� �� ������� 
         * 
         * @param descr ������� �����������.
         */
        Descriptor(const Descriptor& descr) 
            : mscLenght(descr.mscLenght), 
                imsiLenght(descr.imsiLenght), sme(descr.sme)
        {
            setMsc(descr.mscLenght, descr.msc);
            setImsi(descr.imsiLenght, descr.imsi);
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
            setMsc(descr.mscLenght, descr.msc);
            setImsi(descr.imsiLenght, descr.imsi);
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
                msc[mscLenght = _len] = '\0';
            }
            else 
            {
                memset(msc, 0, sizeof(AddressValue));
                mscLenght = 0;
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
                imsi[imsiLenght = _len] = '\0';
            }
            else 
            {
                memset(imsi, 0, sizeof(AddressValue));
                imsiLenght = 0;
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
            
            if (mscLenght)
            {
                memcpy(_value, msc, mscLenght*sizeof(uint8_t));
                _value[mscLenght] = '\0';
            }
            return mscLenght;
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
            
            if (imsiLenght)
            {
                memcpy(_value, imsi, imsiLenght*sizeof(uint8_t));
                _value[imsiLenght] = '\0';
            }
            return imsiLenght;
        }
        
        /**
         * ���������� ������ ������ MSC
         * 
         * @return ������ ������ MSC
         */
        inline uint8_t getMscLenght() const 
        {
            return mscLenght;
        };
        
        /**
         * ���������� ������ ������ IMSI
         * 
         * @return ������ ������ IMSI
         */
        inline uint8_t getImsiLenght() const 
        {
            return imsiLenght;
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
            __require__(_data);

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
         * ���������� ������ ��������������� ���� ��������� 
         * 
         * @return ������ ��������������� ���� ��������� 
         */
        inline uint8_t getDataLenght() const 
        {
            return lenght;
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
        State       state;
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
        uint8_t     messageReference;
        uint8_t     protocolIdentifier;
        
        uint8_t     deliveryReport;
        bool        needArchivate;
        
        uint8_t     failureCause;   // ������� ������� ��������� �������
        uint32_t    attempts;       // ���������� ���������� ������� ��������
        
        Body        messageBody;    // �������������� & ������ ���� ���������
        EService    eServiceType;   

        /**
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         */
        SMS() : state(ENROUTE), attempts(0), failureCause(0) 
        {
            eServiceType[0]='\0';
        }; 
        
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
            originatingDescriptor(sms.originatingDescriptor),
            destinationDescriptor(sms.destinationDescriptor), 
            waitTime(sms.waitTime), validTime(sms.validTime), 
            submitTime(sms.submitTime), lastTime(sms.lastTime),
            nextTime(sms.nextTime), priority(sms.priority),
            messageReference(sms.messageReference),
            protocolIdentifier(sms.protocolIdentifier),
            deliveryReport(sms.deliveryReport),
            needArchivate(sms.needArchivate),
            failureCause(sms.failureCause),
            attempts(sms.attempts),
            messageBody(sms.messageBody) 
        {
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
        };
        
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
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
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
         *
        inline void setNextTime(time_t time) 
        {
            nextTime = time;
        };*/
        
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
        inline void setDeliveryReport(uint8_t report) 
        {
            deliveryReport = report;
        };
        
        /**
         * ���������� �������, ����� �� ����� � �������� ���������
         * 
         * @return �������, ����� �� ����� � �������� ��������� (�� / ���)
         */
        inline uint8_t getDeliveryReport() const 
        {
            return deliveryReport;
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
        
        /**
         * ���������� ���� ���������
         * 
         * @return ���� ���������
         * @see Body
         *
        inline Body& getMessageBody()
        {
            return messageBody; 
        };*/
        
        /**
         * ����� ������������� ���-��� ������� SME.
         * 
         * @param _len   ������ ������ �����-����
         * @param _name ���-��� SME
         */
        inline void setEServiceType(const char* _name) 
        {
            __require__(strlen(_name)<sizeof(EService));
            
            strncpy(eServiceType, _name, sizeof(EService));
        };
        
        /**
         * ����� �������� ���-��� ������� SME � ���������� ��� ������
         * 
         * @param _name ��������� �� ����� ���� ����� ������������ ���
         *              ����� ������ ����� ������ �� ������
         *              MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� 
         *              ����� ��������
         */
        inline void getEServiceType(char* _name) const 
        {
            __require__(_name);
            
            strncpy(_name, eServiceType, sizeof(EService));
        };
    };

}}

#endif


