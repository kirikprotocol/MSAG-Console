//------------------------------------------------------------------------------
//перечисления ошибок
//------------------------------------------------------------------------------
#ifndef MCISME_ADVERT_ADVERTISING_ERRORS
#define MCISME_ADVERT_ADVERTISING_ERRORS

namespace smsc {
namespace mcisme {

enum banner_read_stat
{
  BANNER_OK = 0,
  ERR_ADV_TIMEOUT = 1,    // таймаут сервера 
  ERR_ADV_NOT_CONNECTED = 2,
  ERR_ADV_OTHER = 3,
  CONTINUE_READ_PACKET
};

}}

#endif

