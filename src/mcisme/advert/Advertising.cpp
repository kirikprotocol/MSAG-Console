#include "Advertising.h"

namespace smsc {
namespace mcisme {

uint32_t BannerRequest::lastId = 0;  // ����� ������� 
core::synchronization::Mutex BannerRequest::lastIdMutex; // ������� ��� ������� � ��������

}
}

