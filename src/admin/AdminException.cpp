#include "AdminException.h"

namespace smsc {
namespace admin {

AdminException::AdminException(const AdminException &a)
	: Exception(a.what())
{}

}
}

