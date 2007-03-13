//------------------------------------------------------------------------------
//перечисления ошибок
//------------------------------------------------------------------------------
#ifndef SCAG_ADVERT_ADVERTISING_ERRORS
#define SCAG_ADVERT_ADVERTISING_ERRORS
enum 
{
    ERR_ADV_TIMEOUT = 1,    // таймаут сервера 
    ERR_ADV_SOCKET,         // ошибка сокета
    ERR_ADV_SOCK_WRITE,     // ошибка записи сокета
    ERR_ADV_BANNER_LEN,     // баннер от сервера больше max длины
    ERR_ADV_PACKET_TYPE,    // некорректный тип пакета
    ERR_ADV_PACKET_LEN,     // неверная длина пакета
    ERR_ADV_PACKET_MEMBER,  // неверный элемент пакета
    ERR_ADV_QUEUE_FULL,     // очередь запросов переполнена
    
    ERR_ADV_
};

#endif //SCAG_ADVERT_ADVERTISING_ERRORS

