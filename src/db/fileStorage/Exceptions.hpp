#ifndef SMSC_DB_DBENTITYSTORAGE_EXCEPTIONS
# define SMSC_DB_DBENTITYSTORAGE_EXCEPTIONS 1

# include <db/exceptions/DataSourceExceptions.h>

namespace smsc { namespace db
{
using smsc::util::Exception;

class ElementNotFound : public SQLException
{
public:
  ElementNotFound(const char* msg) : SQLException(msg) {}
  virtual ~ElementNotFound() throw() {}
};

class DupValueException : public SQLException
{
public:
  DupValueException(const char* msg) : SQLException(msg) {}
  virtual ~DupValueException() throw() {}
};

}
}
#endif
