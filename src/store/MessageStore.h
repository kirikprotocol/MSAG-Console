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

    class Iterator
    {
    protected: Iterator() {};
    public:

        virtual bool next()
            throw(StorageException) = 0;
    };

    class IdIterator : public Iterator
    {
    protected: IdIterator() : Iterator() {};
    public:

        virtual SMSId  getId()
            throw(StorageException) = 0;

        virtual bool getNextId(SMSId& id)
            throw(StorageException)
        {
            bool result = next();
            if (result) id = getId();
            return result;
        };
    };

    class TimeIdIterator : public IdIterator
    {
    protected: TimeIdIterator() : IdIterator() {};
    public:

        virtual time_t getTime()
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
         * Замещает тело сообщения SMS в хранилище сообщений.
         *
         * @param id        идентификационный номер сообщения в хранилище
         * @param sms       sms с новым телом
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище.
         * @see SMS
         */
        virtual void replaceSms(SMSId id, SMS& sms)
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
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
                bool skipAttempt=false)
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
         * Изменяет значение счётчика доставленных частей конкатенированного
         * сообщения в хранилище сообщений.
         *
         * @param id     идентификационный номер сообщения в хранилище
         * @param inc    на сколько нужно увеличить счётчик (default = 1)
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @exception NoSuchMessageException
         *                   возникает если сообщение с указанным id
         *                   не существует в хранилище
         *                   или сообщение не является конкатенированным.
         * @see SMS
         */
        virtual void changeSmsConcatSequenceNumber(SMSId id, int8_t inc=1)
                throw(StorageException, NoSuchMessageException) = 0;

        /**
         * Возвращает последний использованный message reference для указанного dda
         * Используется для инициализации механизмов обработки
         * конкатенированных сообщений.
         *
         * @return  последний использованный message reference для указанного dda
         *          если для данного dda не было найдено
         *          конкатенированных  сообщений, то -1
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         */
        virtual int getConcatMessageReference(const Address& dda)
                throw(StorageException) = 0;

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
         * Возвращает итератор над набором id сообщений по адресу получателя.
         * Используется для обработки сообщения от HLR (SME готова к приёму)
         *
         * @param   da      адрес получателя
         *
         * @return итератор над набором id сообщений
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see IdIterator
         */
        virtual IdIterator* getReadyForDelivery(const Address& da)
                throw(StorageException) = 0;

        /**
         * Возвращает итератор над набором id сообщений по составному ключу:
         *  адрес отправителя + адрес получателя + идентификатор сервиса
         *
         * @param   oa      адрес отправителя
         * @param   da      адрес получателя
         * @param   svcType идентификатор сервиса, если нет то 0
         *
         * @return итератор над набором id сообщений
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see IdIterator
         */
        virtual IdIterator* getReadyForCancel(const Address& oa,
            const Address& da, const char* svcType = 0)
                throw(StorageException) = 0;

        /**
         * Возвращает итератор над набором id сообщений
         * с временем следующей попытки доставки, готовых
         * к следующей попытке доставки, т.е. те у которых
         * время следующей попытки доставки не превышает указанного.
         *
         * @param retryTime дата/время для выборки (текущее время)
         * @param immediate признак для сообщений immediate
         * @return итератор над набором id & time сообщений, готовых
         *         к следующей попытке доставки
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see TimeIdIterator
         */
        virtual TimeIdIterator* getReadyForRetry(time_t retryTime, bool immediate=false)
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
