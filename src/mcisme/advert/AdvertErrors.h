//------------------------------------------------------------------------------
//перечисления ошибок
//------------------------------------------------------------------------------
#ifndef MCISME_ADVERT_ADVERTISING_ERRORS
#define MCISME_ADVERT_ADVERTISING_ERRORS

namespace smsc {
namespace mcisme {

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

}}

#endif

