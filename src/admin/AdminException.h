#ifndef SMSC_ADMIN_ADMIN_EXCEPTION
#define SMSC_ADMIN_ADMIN_EXCEPTION

//#include <util/inttypes.h>

namespace smsc {
namespace admin {

class AdminException {
public:
	AdminException(const char *	const message);
	~AdminException();

	const char * const GetMessage() {
		return msg;
	}
protected:
private:
	char * msg;
};

}
}
#endif // ifndef SMSC_ADMIN_ADMIN_EXCEPTION
