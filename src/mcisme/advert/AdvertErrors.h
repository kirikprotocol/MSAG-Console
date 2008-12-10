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
  ERR_ADV_NOT_CONNECTED,
  ERR_ADV_OTHER
};

}}

#endif

