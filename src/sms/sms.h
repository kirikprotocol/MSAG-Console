#ifndef SMS_DECLARATIONS
#define SMS_DECLARATIONS

/**
 * Файл содержит описание внутренней структуры данных для представления SMS
 * в системе SMS центра. Используется системой хранения.
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
    const int MAX_SHORT_MESSAGE_LENGTH = 200;
    
    //const char* DEFAULT_ETSI_GSM_SEVICE_NAME = "GSM-SM";

    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
    typedef uint8_t     SMSData[MAX_SHORT_MESSAGE_LENGTH];
    typedef uint64_t    SMSId;

    /**
     * Структура Address предназначена для хранения 
     * адресов в SMS сообщении.
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
         * Default конструктор, просто инициализирует некоторые поля нулями
         */
        Address() : lenght(1), type(0), plan(0) 
        {
            value[0] = '0'; value[1] = '\0';
        };
        
        /**
         * Конструктор для Address, инициализирует поля структуры реальными данными.
         * Копирует даннуе из буфера к себе
         * 
         * @param _len   длинна буфера _value
         * @param _type  тип адреса
         * @param _plan  план нумерации
         * @param _value значение адреса
         */
        Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
            : lenght(_len), type(_type), plan(_plan) 
        { 
            setValue(_len, _value);
        };
  
        /**
         * Конструктор копирования, используется для создания адреса по образцу
         * 
         * @param addr   образец адреса.
         */
        Address(const Address& addr) 
            : lenght(addr.lenght), type(addr.type), plan(addr.plan) 
        {
            setValue(addr.lenght, addr.value);   
        };

        /**
         * Переопределённый оператор '=',
         * используется для копирования адресов друг в друга
         * 
         * @param addr   Правая часть оператора '='
         * @return Ссылку на себя
         */
        Address& operator =(const Address& addr) 
        {
            type = addr.type; plan = addr.plan; 
            setValue(addr.lenght, addr.value);
            return (*this);
        };
       
        /**
         * Метод устанавливает значение адреса и его длинну.
         * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   длинна нового адреса
         * @param _value значение нового адреса
         */
        inline void setValue(uint8_t _len, const char* _value) 
        {
            __require__(_len && _value && _value[0]
                        && _len<sizeof(AddressValue));
            
            memcpy(value, _value, _len*sizeof(uint8_t));
            value[lenght = _len] = '\0';
        };
        
        /**
         * Метод копирует значение адреса и возвращает его длинну
         * 
         * @param _value указатель на буфер куда будет скопированно значение адреса
         *               буфер должен иметь размер не меньше
         *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
         * @return длинна адреса
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
         * Возвращает длинну адреса
         * 
         * @return длинна адреса
         */
        inline uint8_t getLenght() const 
        {
            return lenght;
        };
       
        /**
         * Устанавливает тип адреса
         * 
         * @param _type  тип адреса
         */
        inline void setTypeOfNumber(uint8_t _type) 
        {
            type = _type;
        };
        
        /**
         * Возвращает тип адреса
         * 
         * @param _type  тип адреса
         */
        inline uint8_t getTypeOfNumber() const 
        { 
            return type; 
        };
      
        /**
         * Устанавливает план нумерации адреса
         * 
         * @param _plan  план нумерации адреса
         */
        inline void setNumberingPlan(uint8_t _plan) 
        {
            plan = _plan;
        };
        
        /**
         * Возвращает план нумерации адреса
         * 
         * @return план нумерации адреса
         */
        inline uint8_t getNumberingPlan() const 
        {
            return plan;
        };
    };
    
    /**
     * Структура Descriptor предназначена для хранения 
     * информации идентифицирующей взаимодействующую сторону
     * (например, источник или приёмник сообщения SMS)
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
         * Default конструктор, просто инициализирует некоторые поля нулями
         */
        Descriptor() : mscLenght(0), imsiLenght(0), sme(0) 
        {
            msc[0] = '\0'; imsi[0] = '\0';
        };
        
        /**
         * Конструктор для Descriptor, инициализирует поля структуры реальными данными.
         * Копирует даннуе из буферов к себе
         * 
         * @param _mscLen   длинна буфера _msc
         * @param _value значение адреса MSC
         * @param _imsiLen   длинна буфера _imsi
         * @param _value значение адреса IMSI
         * @param _sme номер SME
         */
        Descriptor(uint8_t _mscLen, const char* _msc,
                   uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
            : mscLenght(_mscLen), imsiLenght(_imsiLen), sme(_sme)
        { 
            setMsc(mscLenght, _msc);
            setImsi(imsiLenght, _imsi);
        };
  
        /**
         * Конструктор копирования, используется для
         * создания дескриптора по образцу 
         * 
         * @param descr образец дескриптора.
         */
        Descriptor(const Descriptor& descr) 
            : mscLenght(descr.mscLenght), 
                imsiLenght(descr.imsiLenght), sme(descr.sme)
        {
            setMsc(descr.mscLenght, descr.msc);
            setImsi(descr.imsiLenght, descr.imsi);
        };

        /**
         * Переопределённый оператор '=',
         * используется для копирования дескрипторов друг в друга
         * 
         * @param descr   Правая часть оператора '='
         * @return Ссылку на себя
         */
        Descriptor& operator =(const Descriptor& descr) 
        {
            sme = descr.sme;
            setMsc(descr.mscLenght, descr.msc);
            setImsi(descr.imsiLenght, descr.imsi);
            return (*this);
        };
       
        /**
         * Метод устанавливает значение адреса MSC и его длинну.
         * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   длинна нового адреса MSC
         * @param _value значение нового адреса MSC
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
         * Метод устанавливает значение адреса IMSI и его длинну.
         * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
         * 
         * @param _len   длинна нового адреса IMSI
         * @param _value значение нового адреса IMSI
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
         * Метод копирует значение адреса MSC и возвращает его длинну
         * 
         * @param _value указатель на буфер куда будет скопированно значение 
         *               адреса MSC. Буфер должен иметь размер не меньше
         *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
         * @return длинна адреса MSC
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
         * Метод копирует значение адреса IMSI и возвращает его длинну
         * 
         * @param _value указатель на буфер куда будет скопированно значение 
         *               адреса IMSI. Буфер должен иметь размер не меньше
         *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
         * @return длинна адреса IMSI
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
         * Возвращает длинну адреса MSC
         * 
         * @return длинна адреса MSC
         */
        inline uint8_t getMscLenght() const 
        {
            return mscLenght;
        };
        
        /**
         * Возвращает длинну адреса IMSI
         * 
         * @return длинна адреса IMSI
         */
        inline uint8_t getImsiLenght() const 
        {
            return imsiLenght;
        };
        
        /**
         * Устанавливает номер SME
         * 
         * @return длинна адреса IMSI
         */
        inline void setSmeNumber(uint32_t _sme)
        {
            sme = _sme;
        };
        
        /**
         * Возвращает номер SME
         * 
         * @return номер SME
         */
        inline uint32_t getSmeNumber() const 
        {
            return sme;
        };
    };

    /**
     * Структура описывает тело сообщения
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
         * Default конструктор, просто инициализирует некоторые поля нулями
         */
        Body() : header(false), scheme(0), lenght(0)
        {
            //memset((void *)data, 0, sizeof(data));
            data[0]='\0';
        };
        
        /**
         * Конструктор для Body, инициализирует поля структуры реальными данными.
         * Копирует даннуе из буфера к себе
         * 
         * @param _len    длинна буфера _data
         * @param _scheme схема кодирования тела сообщения
         * @param _header идентифицирует содержит ли тело заголовок
         * @param _data   закодированное тело сообщения
         */
        Body(uint8_t _len, uint8_t _scheme, bool _header, const uint8_t* _data)
            : header(_header), scheme(_scheme), lenght(_len)
        { 
            setData(_len, _data);
        };
        
        /**
         * Конструктор копирования, используется для создания тела по образцу
         * 
         * @param body   образец тела.
         */
        Body(const Body& body) 
            : header(body.header), scheme(body.scheme), lenght(body.lenght)
        {
            setData(body.lenght, body.data);   
        };
        
        /**
         * Переопределённый оператор '=',
         * используется для копирования тел сообщений друг в друга
         * 
         * @param body   Правая часть оператора '='
         * @return ссылку на себя
         */
        Body& operator =(const Body& body) 
        {
            header = body.header; scheme = body.scheme;
            setData(body.lenght, body.data);
            return (*this);
        };

        /**
         * Метод устанавливает значение тела и его длинну.
         * Длинна тела должна быть меньше либо равна MAX_SMS_DATA_LENGTH.
         * 
         * @param _len   длинна нового тела
         * @param _value значение нового тела
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
         * Метод копирует значение тела и возвращает его длинну
         * 
         * @param _data  указатель на буфер куда будет скопированно значение тела
         *               буфер должен иметь размер не меньше
         *               MAX_SMS_DATA_LENGTH, чтобы принять любое значение
         * @return длинна тела
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
         * Устанавливает схему кодирования тела сообщения
         * 
         * @param _scheme схема кодирования тела сообщения
         */
        inline void setCodingScheme(uint8_t _scheme) 
        {
            scheme = _scheme;
        };
        
        /**
         * Возвращает схему кодирования тела сообщения
         * 
         * @return схема кодирования тела сообщения
         */
        inline uint8_t getCodingScheme() const 
        {
            return scheme;
        };
        
        /**
         * Устанавливает признак наличия заголовка в теле сообщения
         * 
         * @param _header признак наличия заголовка в теле сообщения
         */
        inline void setHeaderIndicator(bool _header) 
        {
            header = _header;
        };
        
        /**
         * Проверяет установлен ли признак наличия заголовка в теле сообщения
         * 
         * @return да / нет
         */
        inline bool isHeaderIndicator() const 
        {
            return header;
        };

        /**
         * Возвращает длинну закодированного тела сообщения 
         * 
         * @return длинна закодированного тела сообщения 
         */
        inline uint8_t getDataLenght() const 
        {
            return lenght;
        };

        /**
         * Метод декодирует тело сообщения. 
         * Выделяет память, после использования её необходимо освободить.
         * Пока не имплементированно
         * 
         * @return декодированное тело сообщения. Сейчас NULL.
         */
        char* getDecodedText();

				uint8_t* getBuffer();
				int getBufferLength();
				void setBuffer(uint8_t* buffer,int length);
    };
   
    const uint8_t SMSC_BYTE_ENROUTE_STATE       = (uint8_t)0;
    const uint8_t SMSC_BYTE_DELIVERED_STATE     = (uint8_t)1;
    const uint8_t SMSC_BYTE_EXPIRED_STATE       = (uint8_t)2;
    const uint8_t SMSC_BYTE_UNDELIVERABLE_STATE = (uint8_t)3;
    const uint8_t SMSC_BYTE_DELETED_STATE       = (uint8_t)4;

    /**
     * Множество состояний SMS в контексте SMS центра
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
     * Структура описывающая SMS в контексте операций
     * SMS ценра. Используется системой хранения.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see MessageStore
     */
    struct SMS 
    {
        State       state;
        uint16_t    messageReference;
        
        Address     originatingAddress;
        Address     destinationAddress;
        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;
        
        time_t      waitTime;       // Время/Дата с которого пытаться доставить
        time_t      validTime;      // Время/Дата до которого сообщение валидно
        time_t      submitTime;     // Время/Дата поступления на SMSC
        time_t      lastTime;       // Время/Дата последней попытки доставки
        time_t      nextTime;       // Время/Дата слудующей попытки доставки
        
        uint8_t     priority;
        uint8_t     protocolIdentifier;
        
        uint8_t     deliveryReport;
        bool        needArchivate;
        
        uint8_t     failureCause;   // Причина неудачи последней попытки
        uint32_t    attempts;       // Количество неуспешных попыток доставки
        
        Body        messageBody;    // Закодированное & сжатое тело сообщения
        EService    eServiceType;   

        SMSId       receiptSmsId;   // id сообщения на который идёт ответ-репорт
        uint8_t     esmClass;       // тип сообщения: нормальное, ответ ...

        /**
         * Default конструктор, просто инициализирует поле state как ENROUTE
         */
        SMS() : state(ENROUTE), lastTime(0), nextTime(0),
                failureCause(0), attempts(0), receiptSmsId(0), esmClass(0)
        {
            eServiceType[0]='\0';
        }; 
        
        /**
         * Конструктор копирования, используется для создания
         * SMS по образцу
         * 
         * @param sms    образец SMS
         */
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
        };
        
        /**
         * Переопределённый оператор '=',
         * используется для копирования сообщений
         * 
         * @param sms   Правая часть оператора '='
         * @return ссылку на себя
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
            receiptSmsId = sms.receiptSmsId;
            esmClass = sms.esmClass;
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
            return (*this);
        };
        
        /**
         * Возвращает состояние сообщения
         * 
         * @return состояние сообщения
         */
        inline State getState() const 
        {
            return state;
        };

        /**
         * Устанавливает адрес отправителя.
         * Копирует адрес во внутренние структуры
         * 
         * @param lenght длинна адреса (0 < lenght <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   тип адреса
         * @param plan   план нумерации адреса
         * @param buff   значение адреса
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
         * Устанавливает адрес отправителя
         * Копирует адрес во внутренние структуры
         * 
         * @param address новый адрес отправителя
         * @see Address
         */
        inline void setOriginatingAddress(const Address& address) 
        { // Copies address value from 'address' to static structure 
            originatingAddress = address;     
        };
        
        /**
         * Возвращает адрес отправителя
         * 
         * @return адрес отправителя
         * @see Address
         */
        inline const Address& getOriginatingAddress() const 
        {
            return originatingAddress; 
        };
        
        /**
         * Возвращает адрес отправителя
         * 
         * @return адрес отправителя
         * @see Address
         */
        inline Address& getOriginatingAddress()
        {
            return originatingAddress; 
        };

        /**
         * Устанавливает адрес получателя
         * Копирует адрес во внутренние структуры
         * 
         * @param lenght длинна адреса (0 < lenght <= MAX_ADDRESS_VALUE_LENGTH)
         * @param type   тип адреса
         * @param plan   план нумерации адреса
         * @param buff   значение адреса
         * @see Address
         */
        inline void setDestinationAddress(uint8_t lenght, uint8_t type, 
                                          uint8_t plan, const char* buff) 
        { // Copies address value from 'buff' to static structure 
            destinationAddress.setTypeOfNumber(type);
            destinationAddress.setNumberingPlan(plan);
            destinationAddress.setValue(lenght, buff);
        };
        
        /**
         * Устанавливает адрес получателя
         * Копирует адрес во внутренние структуры
         * 
         * @param address новый адрес получателя
         * @see Address
         */
        inline void setDestinationAddress(const Address& address) 
        { // Copies address value from 'address' to static structure 
            destinationAddress = address;     
        };
        
        /**
         * Возвращает адрес получателя
         * 
         * @return адрес получателя
         * @see Address
         */
        inline const Address& getDestinationAddress() const
        {
            return destinationAddress; 
        };
        
        /**
         * Возвращает адрес получателя
         * 
         * @return адрес получателя
         * @see Address
         */
        inline Address& getDestinationAddress()
        {
            return destinationAddress; 
        };
       
        /**
         * Устанавливает дескриптор отправителя
         * Копирует адрес во внутренние структуры
         * 
         * @param descriptor новый дескриптор отправителя
         * @see Descriptor
         */
        inline void setOriginatingDescriptor(const Descriptor& descriptor) 
        { // Copies descriptor from 'descriptor' to static structure 
            originatingDescriptor = descriptor;     
        };
        
        /**
         * Устанавливает дескриптор отправителя
         * Копирует адрес во внутренние структуры
         *
         * @param _mscLen   длинна буфера _msc
         * @param _value значение адреса MSC
         * @param _imsiLen   длинна буфера _imsi
         * @param _value значение адреса IMSI
         * @param _sme номер SME
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
         * Возвращает дескриптор отправителя
         * 
         * @return дескриптор отправителя
         * @see Descriptor
         */
        inline const Descriptor& getOriginatingDescriptor() const 
        {
            return originatingDescriptor; 
        };
        
        /**
         * Возвращает дескриптор отправителя
         * 
         * @return дескриптор отправителя
         * @see Descriptor
         */
        inline Descriptor& getOriginatingDescriptor()
        {
            return originatingDescriptor; 
        };
        
        /**
         * Устанавливает дескриптор получателя
         * Копирует адрес во внутренние структуры
         * 
         * @param descriptor новый дескриптор получателя
         * @see Descriptor
         *
        inline void setDestinationDescriptor(const Descriptor& descriptor) 
        { // Copies descriptor from 'descriptor' to static structure 
            destinationDescriptor = descriptor;     
        };*/
        
        /**
         * Возвращает дескриптор получателя
         * 
         * @return дескриптор получателя
         * @see Descriptor
         */
        inline const Descriptor& getDestinationDescriptor() const 
        {
            return destinationDescriptor; 
        };
        
        /**
         * Возвращает дескриптор получателя
         * 
         * @return дескриптор получателя
         * @see Descriptor
         *
        inline Descriptor& getDestinationDescriptor()
        {
            return destinationDescriptor; 
        };*/
        

        /**
         * Устанавливает время ожидания. 
         * 
         * @param time   дата, когда сообщение должно быть отправлено
         *               (не интервал ожидания).
         */
        inline void setWaitTime(time_t time) 
        {
            waitTime = time;
        };
        
        /**
         * Возвращает время ожидания.
         * 
         * @return дата, когда сообщение должно быть отправлено
         *         (не интервал ожидания).
         */
        inline time_t getWaitTime() const 
        {
            return waitTime;
        };
        
        /**
         * Устанавливает время валидности сообщения. 
         * 
         * @param time   дата, до которой сообщение валидно
         *               (не интервал времени).
         */
        inline void setValidTime(time_t time) 
        {
            validTime = time;
        };
        
        /**
         * Возвращает время валидности сообщения.
         * 
         * @return time   дата, до которой сообщение валидно
         *         (не интервал времени).
         */
        inline time_t getValidTime() const 
        {
            return validTime;
        };
        
        /**
         * Устанавливает время поступления сообщения в SMSC
         * 
         * @param time   время поступления сообщения в SMSC
         */
        inline void setSubmitTime(time_t time) 
        {
            submitTime = time;
        };
        
        /**
         * Возвращает время поступления сообщения в SMSC
         * 
         * @return время поступления сообщения в SMSC
         */
        inline time_t getSubmitTime() const 
        {
            return submitTime;
        };
        
        /**
         * Устанавливает время последней попытки доставки сообщения из SMSC
         * 
         * @param time   время последней попытки доставки сообщения из SMSC
         *
        inline void setLastTime(time_t time) 
        {
            lastTime = time;
        };*/
        
        /**
         * Возвращает время последней попытки доставки сообщения из SMSC
         * 
         * @return время последней попытки доставки сообщения из SMSC
         */
        inline time_t getLastTime() const 
        {
            return lastTime;
        };
        
        /**
         * Устанавливает время следующей попытки доставки сообщения из SMSC
         * 
         * @param time   время следующей попытки доставки сообщения из SMSC
         */
        inline void setNextTime(time_t time) 
        {
            nextTime = time;
        };
        
        /**
         * Возвращает время следующей попытки доставки сообщения из SMSC
         * 
         * @return время следующей попытки доставки сообщения из SMSC
         */
        inline time_t getNextTime() const 
        {
            return nextTime;
        };
        
        /**
         * Устанавливает номер сообщения (MR), 
         * для идентификации сообщений приходящих с одного адреса
         * 
         * @param mr     идентификационный номер сообщения (MR)
         */
        inline void setMessageReference(uint16_t mr) 
        {
            messageReference = mr;
        };
        
        /**
         * Возвращает номер сообщения (MR),
         * для идентификации сообщений приходящих с одного адреса
         * 
         * @return идентификационный номер сообщения (MR)
         */
        inline uint16_t getMessageReference() const 
        {
            return messageReference;
        };
       
        /**
         * Устанавливает приоритет сообщения.
         * В чистом стандарте SMS не используется, но нужен для SMPP
         * 
         * @param pri    приоритет сообщения
         */
        inline void setPriority(uint8_t pri) 
        {
            priority = pri;
        };
        
        /**
         * Возвращает приоритет сообщения.
         * В чистом стандарте SMS не используется, но нужен для SMPP
         * 
         * @return приоритет сообщения
         */
        inline uint8_t getPriority() const 
        {
            return priority;
        };
       
        /**
         * Устанавливает прокол передачи сообщения.
         * В чистом стандарте SMS не используется, но нужен для SMPP
         * 
         * @param pid    прокол передачи сообщения
         */
        inline void setProtocolIdentifier(uint8_t pid) 
        {
            protocolIdentifier = pid;
        };
        
        /**
         * Возвращает прокол передачи сообщения.
         * В чистом стандарте SMS не используется, но нужен для SMPP
         * 
         * @return прокол передачи сообщения
         */
        inline uint8_t getProtocolIdentifier() const 
        {
            return protocolIdentifier;
        };
       
        /**
         * Устанавливает признак, нужен ли отчет о доставке сообщения 
         * 
         * @param req    признак, нужен ли отчет о доставке сообщения
         */
        inline void setDeliveryReport(uint8_t report) 
        {
            deliveryReport = report;
        };
        
        /**
         * Возвращает признак, нужен ли отчет о доставке сообщения
         * 
         * @return признак, нужен ли отчет о доставке сообщения (да / нет)
         */
        inline uint8_t getDeliveryReport() const 
        {
            return deliveryReport;
        };
       
        /**
         * Устанавливает признак, нужно ли архивировать сообщение после 
         * успешной или неуспешной попытке доставки 
         * 
         * @param arc    признак, нужно ли архивировать сообщение 
         */
        inline void setArchivationRequested(bool arc) 
        {
            needArchivate = arc;
        };
        
        /**
         * Возвращает признак, нужно ли архивировать сообщение после 
         * успешной или неуспешной попытке доставки 
         * 
         * @return признак, нужно ли нужно ли архивировать сообщение (да / нет)
         */
        inline bool isArchivationRequested() const 
        {
            return needArchivate;
        };
       
        /**
         * Устанавливает причину в случае 
         * отказа/некорректности/недоставки сообщения
         * 
         * @param cause  причина отказа/некорректности/недоставки сообщения
         *
        inline void setFailureCause(uint8_t cause) 
        {
            failureCause = cause;
        };*/
        
        /**
         * Возвращает причину в случае
         * отказа/некорректности/недоставки сообщения
         * 
         * @return причина отказа/некорректности/недоставки сообщения
         */
        inline uint8_t getFailureCause() const 
        {
            return failureCause;
        };
        
        /**
         * Устанавливает количество неудач при доставке сообщения
         * 
         * @param count  количество неудач при доставке сообщения
         *
        inline void setAttemptsCount(uint32_t count) 
        {
            attempts = count;
        };*/
        
        /**
         * Увеличивает количество неудач при доставке сообщения 
         * на указанную величину (по умолчанию 1)
         * 
         * @param count  количество неудач при доставке сообщения
         *
        inline void incrementAttemptsCount(uint32_t count=1) 
        {
            attempts += count;
        };*/
        
        /**
         * Возвращает количество неудач при доставке сообщения 
         * 
         * @return количество неудач при доставке сообщения 
         */
        inline uint32_t getAttemptsCount() const 
        {
            return attempts;
        };
        
        /**
         * Устанавливает тело сообщения
         * 
         * @param lenght длинна тела сообщения
         * @param scheme схема кодировки тела сообщения
         * @param header признак, содержит ли тело заголовок
         * @param buff   закодированные данные в теле сообщения 
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
         * Устанавливает тело сообщения
         * 
         * @param body тело сообщения
         * @see Body
         */
        inline void setMessageBody(const Body& body) 
        { // Copies body data from 'body' to static structure 
            messageBody = body;     
        };
        
        /**
         * Возвращает тело сообщения
         * 
         * @return тело сообщения
         * @see Body
         */
        inline const Body& getMessageBody() const
        {
            return messageBody; 
        };
        
        /**
         * Возвращает тело сообщения
         * 
         * @return тело сообщения
         * @see Body
         */
        inline Body& getMessageBody()
        {
            return messageBody; 
        };
        
        /**
         * Метод устанавливает имя-тип сервиса SME.
         * 
         * @param _len   длинна нового имени-типа
         * @param _name имя-тип SME
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
         * Метод копирует имя-тип сервиса SME и возвращает его длинну
         * 
         * @param _name указатель на буфер куда будет скопированно имя
         *              буфер должен иметь размер не меньше
         *              MAX_ESERVICE_TYPE_LENGTH+1, чтобы принять 
         *              любое значение
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
         * Устанавливает id сообщения, на которое идёт ответ-репорт.
         * Используется в случае если esmClass это ответ-репорт.
         * 
         * @param id     идентификационный номер сообщения
         */
        inline void setReceiptSmsId(SMSId id) 
        {
            receiptSmsId = id;
        };
        
        /**
         * Возвращает id сообщения, на которое идёт ответ-репорт.
         * Используется в случае если esmClass это ответ-репорт.
         * 
         * @return идентификационный номер сообщения (MR)
         */
        inline SMSId getReceiptSmsId() const 
        {
            return receiptSmsId;
        };
        
        /**
         * Устанавливает тип-сообщения.
         * 
         * @param type      тип-сообщения    
         */
        inline void setEsmClass(uint8_t type) 
        {
            esmClass = type;
        };
        
        /**
         * Возвращает тип-сообщения.
         * 
         * @return тип-сообщения
         */
        inline uint8_t getEsmClass() const 
        {
            return esmClass;
        };
    
    };

}}

#endif


