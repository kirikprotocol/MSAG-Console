#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

/**
 * Файл содержит описание интерфейса подсистемы хранения сообщений
 * в контексте SMS центра.
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see SMS
 * @see StoreManager
 */

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    class IdIterator
    {
    protected:

        /**
         * Защищённый конструктор.
         * Экземпляр интерфейса IdIterator может быть создан
         * только через производный класс, например ReadyIdIterator
         *
         * @see StoreManager::ReadyIdIterator
         */
        IdIterator() {};
    
    public:

        /**
         * Деструктор, уничтожает курсор и освобождает соединение
         * с хранилищем. Должен быть реализован производным классом,
         * например ReadyIdIterator
         *
         * @see StoreManager::ReadyIdIterator
         */
        virtual ~IdIterator() {};

        /**
         * Используется для получения следующего id'а сообщения.
         * Реализуется производным классом, например ReadyIdIterator.
         * 
         * @param id     ссылка для возврата id в случае успеха
         * @return признак, был ли извлечён id или больше id'ов нет
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         *                   
         * @see StoreManager::ReadyIdIterator
         */
        virtual bool getNextId(SMSId& id)
            throw(StorageException) = 0;
    };

    /**
     * Допустимые режимы создания SMS в контексте вызова createSMS()
     *
     * CREATE_NEW                -  всегда создавать новое сообщение в БД;
     * SMPP_OVERWRITE_IF_PRESENT -  параметр указывающий переписывать
     *                              сообщение целиком если оно было
     *                              найдено по OA, DA & SVC_TYPE;
     * ETSI_REJECT_IF_PRESENT    -  параметр указывающий отказать в
     *                              создании сообщения если есть сообщение
     *                              с аналогичными OA, DA & MR.
     *
     * @see MessageStore::createSMS()
     */
    typedef enum { 
        CREATE_NEW=0, SMPP_OVERWRITE_IF_PRESENT=1, ETSI_REJECT_IF_PRESENT=2 
    } CreateMode;
    
    /**
     * Интерфейс подсистемы хранения сообщений в контексте SMS центра.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see SMS
     * @see StoreManager
     */
    class MessageStore 
    {
    public:    
        
        /**
         * Возвращает следующий id для создания SMS в хранилище
         * 
         * @return следующий id для создания SMS в хранилище
         */
        virtual SMSId getNextId() = 0;
        
        /**
         * Помещает SMS в состоянии ENROUTE в хранилище сообщений.
         * Необходимо чтобы сохраняемая sms была корректно
         * проинициализированна через весь набор своих setter'ов.
         * 
         * @param sms    SMS которую нужно поместить в хранилище.
         * @param flag   флаг-режим сохранения, default - CREATE_NEW
         * @return id сообщения в хранилище
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception DuplicateMessageException
         *                   возникает при установленном флаге
         *                   ETSI_REJECT_IF_PRESENT и совпадении OA, DA & MR
         *                   у некоторого существующего сообщения в хранилище
         *                   в состоянии ENROUTE.
         * @see CreateMode
         * @see SMS
         */
        virtual SMSId createSms(SMS& sms, SMSId id,
                               const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException) = 0;
        
        /**
         * Извлекает SMS из хранилища сообщений.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @param sms    структура SMS для заполнения извлечённой записью.
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException) = 0;
        
        /**
         * Замещает тело сообщения SMS в хранилище сообщений.
         * Не меняет схему кодирования сообщения и признак наличия заголовка (?)
         * 
         * @param id        идентификационный номер сообщения в хранилище
         * @param oa        структура-адрес отправителя
         * @param newMsg    новое тело сообщения
         * @param newMsgLen длинна нового тела сообщения
         * @param deliveryReport
         *                  новый признак отчёта о доставке сообщения
         * @param validTime новая дата/время до которого сообщение валидно
         *                  если не требуется, то нужно установить в 0
         * @param waitTime  новая дата/время для начала попыток доставки,
         *                  если не требуется, то нужно установить в 0
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t nextTime = 0)
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * Изменяет аттрибуты SMS в хранилище сообщений.
         * Используется в случае неуспешной попытки доставки.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @param dst    структура Descriptor, описывающая адреса
         *               MSC, IMSI и номер SME получателя SMS.
         * @param failureCause
         *               причина неудачи при попытке доставки
         * @param nextTryTime
         *               дата/время следующей попытки доставки
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * Изменяет состояние в DELIVERED и аттрибуты SMS в хранилище сообщений.
         * Используется в случае успешной доставки сообщения.
         * Также обнуляет причину последней неудачной попытки.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @param dst    структура Descriptor, описывающая адреса
         *               MSC, IMSI и номер SME получателя SMS.
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void changeSmsStateToDelivered(SMSId id, 
            const Descriptor& dst) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * Изменяет состояние в UNDELIVERABLE и аттрибуты SMS
         * в хранилище сообщений.
         * Используется в случае принципиальной невозможности доставки
         * сообщения. Т.н., если абонента с таким адресом не существует.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @param dst    структура Descriptor, описывающая адреса
         *               MSC, IMSI и номер SME получателя SMS.
         * @param failureCause
         *               причина неудачи при попытке доставки
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * Изменяет состояние SMS в EXPIRED в хранилище сообщений.
         * Используется в случае истечения срока жизни сообщения (validTime).
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void changeSmsStateToExpired(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
    
        /**
         * Изменяет состояние SMS в DELETED в хранилище сообщений.
         * Используется в случае запроса пользователя на удаление сообщения.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void changeSmsStateToDeleted(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * Удаляет сообщение SMS из хранилища сообщений.
         * Используется только для внутренних нужд тестирования системы.
         * 
         * @param id     идентификационный номер сообщения в хранилище
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void destroySms(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0;
    
        /**
         * Возвращает итератор над набором id сообщений, готовых
         * к следующей попытке доставки, т.е. те у которых
         * время следующей попытки доставки не превышает указанного.
         * 
         * @param retryTime дата/время для выборки (текущее время)
         * @return итератор над набором id сообщений, готовых
         *         к следующей попытке доставки
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see IdIterator
         */
        virtual IdIterator* getReadyForRetry(time_t retryTime) 
                throw(StorageException) = 0;
        
        /**
         * Возвращает минимальное время для следующей попытки
         * доставки сообщений находящихся в хранилище.
         * В случае отсутствия сообщений, ожидающих следующей 
         * попытки доставки, возвращает 0.
         * 
         * @return минимальное время для следующей попытки
         *         доставки сообщений находящихся в хранилище
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         */
        virtual time_t getNextRetryTime() 
                throw(StorageException) = 0;
    
    protected:
        
        /**
         * Защищённый конструктор.
         * Экземпляр интерфейса MessageStore может быть создан
         * только с помощью StoreManager
         *
         * @see StoreManager
         */
        MessageStore() {};

        /**
         * Защищённый деструктор.
         * Экземпляр интерфейса MessageStore может быть уничтожен
         * только с помощью StoreManager
         *
         * @see StoreManager
         */
        virtual ~MessageStore() {};
    };
}}

#endif

