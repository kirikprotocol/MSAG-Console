#ifndef SMSC_ADMIN_SERVICE_VARIANT
#define SMSC_ADMIN_SERVICE_VARIANT

#include <admin/AdminException.h>
#include <admin/service/Type.h>
#include <core/buffers/Hash.hpp>
#include <list>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

typedef std::list<const char *> StringList;

class Variant
{
private:
	Type type;
	union {
		char * strValue;
		long longValue;
		bool boolValue;
    StringList* stringListValue;
	};
public:
	Variant();
  Variant(const Type type) throw (AdminException);
	Variant(const char * const value);
	Variant(const long value);
	Variant(const bool value);
  Variant(const StringList & value);

  Variant(const char * const value, Type type) throw (AdminException);
	Variant(const Variant &copy) throw (AdminException);

  virtual ~Variant();

	Variant & operator = (const Variant & copy) throw (AdminException);


	const Type getType() const;
	const char * const getStringValue() const throw (AdminException);
	const long getLongValue() const throw (AdminException);
	const bool getBooleanValue() const throw (AdminException);
  const StringList& getStringListValue() const throw (AdminException);

	void appendValueToStringList(const char * const value) throw (AdminException);


private:
	void initByCopy(const Variant& copy) throw (AdminException);
};

typedef smsc::core::buffers::Hash<Variant> Arguments;

}
}
}

#endif //ifndef SMSC_ADMIN_SERVICE_VARIANT
