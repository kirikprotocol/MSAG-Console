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
#include <orl.h>

#include <util/debug.h>

namespace smsc { namespace sms
{
    const int MAX_ADDRESS_VALUE_LENGTH = 21;
    const int MAX_SHORT_MESSAGE_LENGTH = 200;
    
    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
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
        Address() : lenght(0), type(0), plan(0) 
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
         * Метод копирует значение адреса и возвращает его длинну
         * 
         * @param _value указатель на буфер куда будет скопированно значение адреса
         *               буфер должен иметь размер не меньше
         *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
         * @return длинна адреса
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
        Body() : header(false), lenght(0), scheme(0) 
        {
            //memset((void *)data, 0, sizeof(data));
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
            : header(_header), lenght(_len), scheme(_scheme) 
        { 
            setData(_len, _data);
        };
        
        /**
         * Конструктор копирования, используется для создания тела по образцу
         * 
         * @param body   образец тела.
         */
        Body(const Body& body) 
            : header(body.header), lenght(body.lenght), scheme(body.scheme)
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
            __require__(data);

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
         * Метод декодирует тело сообщения. 
         * Выделяет память, после использования её необходимо освободить.
         * Пока не имплементированно
         * 
         * @return декодированное тело сообщения. Сейчас NULL.
         */
        char* getDecodedText();
    };
   
    /**
     * Множество состояний SMS в контексте SMS центра
     * 
     * @see SMS
     */
    typedef enum { 
        ENROUTE=0, DELIVERED=1, 
        EXPIRED=2, UNDELIVERABLE=3,
        DELETED=4 
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
         * Default конструктор, просто инициализирует поле state как ENROUTE
         */
        SMS() : state(ENROUTE) {}; 
        
        /**
         * Конструктор копирования, используется для создания
         * SMS по образцу
         * 
         * @param sms    образец SMS
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
         * Устанавливает состояние сообщения
         * 
         * @param state  новое состояние
         */
        inline void setState(State state) 
        {
            this->state = state;
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
            __require__(lenght);

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
         */
        inline void setDeliveryTime(time_t time) 
        {
            deliveryTime = time;
        };
        
        /**
         * Возвращает время последней попытки доставки сообщения из SMSC
         * 
         * @return время последней попытки доставки сообщения из SMSC
         */
        inline time_t getDeliveryTime() const 
        {
            return deliveryTime;
        };
        
        /**
         * Устанавливает номер сообщения (MR), 
         * для идентификации сообщений приходящих с одного адреса
         * 
         * @param mr     идентификационный номер сообщения (MR)
         */
        inline void setMessageReference(uint8_t mr) 
        {
            messageReference = mr;
        };
        
        /**
         * Возвращает номер сообщения (MR),
         * для идентификации сообщений приходящих с одного адреса
         * 
         * @return идентификационный номер сообщения (MR)
         */
        inline uint8_t getMessageReference() const 
        {
            return messageReference;
        };
       
        /**
         * Устанавливает номер сообщения (SMI), 
         * для идентификации сообщений исходящих на один адрес
         * ! Скорре всего не нужен !
         * 
         * @param mi     идентификационный номер сообщения (SMI)
         */
        inline void setMessageIdentifier(uint8_t mi) 
        {
            messageIdentifier = mi;
        };
        
        /**
         * Возвращает номер сообщения (SMI),
         * для идентификации сообщений исходящих на один адрес
         * ! Скорре всего не нужен !
         * 
         * @return идентификационный номер сообщения (SMI)
         */
        inline uint8_t getMessageIdentifier() const 
        {
            return messageIdentifier;
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
        inline void setStatusReportRequested(bool req) 
        {
            statusReportRequested = req;
        };
        
        /**
         * Возвращает признак, нужен ли отчет о доставке сообщения
         * 
         * @return признак, нужен ли отчет о доставке сообщения (да / нет)
         */
        inline bool isStatusReportRequested() const 
        {
            return statusReportRequested;
        };
       
        /**
         * Устанавливает признак, нужно ли отсекать сообщения дубликаты
         * (с одинаковыми MR с одного адреса) 
         * 
         * @param rej    признак, нужно ли отсекать сообщения дубликаты 
         */
        inline void setRejectDuplicates(bool rej) 
        {
            rejectDuplicates = rej;
        };
        
        /**
         * Возвращает признак, нужно ли отсекать сообщения дубликаты
         * (с одинаковыми MR с одного адреса) 
         * 
         * @return признак, нужно ли отсекать сообщения дубликаты (да / нет)
         */
        inline bool isRejectDuplicates() const 
        {
            return rejectDuplicates;
        };
       
        /**
         * Устанавливает причину в случае 
         * отказа/некорректности/недоставки сообщения
         * 
         * @param cause  причина отказа/некорректности/недоставки сообщения
         */
        inline void setFailureCause(uint8_t cause) 
        {
            failureCause = cause;
        };
        
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
    };

}}

#endif


