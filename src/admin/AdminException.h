#ifndef SMSC_ADMIN_ADMIN_EXCEPTION
#define SMSC_ADMIN_ADMIN_EXCEPTION

#include <exception>

namespace smsc {
namespace admin {

class AdminException : public std::exception
{
public:
//	AdminException(AdminException a);
	AdminException(const AdminException &a) throw();
	AdminException(const char *	const message) throw();
	~AdminException() throw ();

	const char * what() const throw()
	{
		return msg;
	}
protected:
private:
	char * msg;
};

}
}
#endif // ifndef SMSC_ADMIN_ADMIN_EXCEPTION
