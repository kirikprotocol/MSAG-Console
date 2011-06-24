#include "Advertising.h"

namespace smsc {
namespace mcisme {

uint32_t BannerRequest::lastId = 0;  // общий счетчик 
core::synchronization::Mutex BannerRequest::lastIdMutex; // мьютекс для доступа к счетчику

}
}

