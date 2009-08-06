//------------------------------------
//  ProfilesStorage.cpp
//  Routman Michael, 2006
//------------------------------------

#include <mcisme/ProfilesStorage.hpp>

namespace smsc { namespace mcisme
{

using namespace core::synchronization;
using namespace core::buffers;

ProfilesStorage* ProfilesStorage::pInstance=0;
AddrDiskHash ProfilesStorage::profiles;
Mutex   ProfilesStorage::mutex;
bool    ProfilesStorage::_isOpen = false;

Logger* ProfilesStorage::logger = 0;

bool ProfilesStorage::_needLogstore = false;

};	//  namespace msisme
};	//  namespace smsc

