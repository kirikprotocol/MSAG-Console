#ifndef __DBENTITYSTORAGE_DBENTITYSTORAGESTATEMENT_HPP__
# define __DBENTITYSTORAGE_DBENTITYSTORAGESTATEMENT_HPP__ 1

# include <db/exceptions/DataSourceExceptions.h>
# include <db/DataSource.h>

using smsc::util::Exception;
using smsc::db::ResultSet;
using smsc::db::Statement;
using smsc::db::InvalidArgumentException;
using smsc::db::SQLException;

class DBEntityStorageResultSet : public ResultSet
{
public:
  virtual ~DBEntityStorageResultSet() {}

  virtual bool fetchNext()
    throw(SQLException);

  virtual bool isNull(int pos)
    throw(SQLException, InvalidArgumentException);

  virtual const char* getString(int pos)
    throw(SQLException, InvalidArgumentException);

  virtual int8_t getInt8(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual int16_t getInt16(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual int32_t getInt32(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual int64_t getInt64(int pos)
    throw(SQLException, InvalidArgumentException);

  virtual uint8_t getUint8(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual uint16_t getUint16(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual uint32_t getUint32(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual uint64_t getUint64(int pos)
    throw(SQLException, InvalidArgumentException);

  virtual float getFloat(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual double getDouble(int pos)
    throw(SQLException, InvalidArgumentException);
  virtual long double getLongDouble(int pos)
    throw(SQLException, InvalidArgumentException);

  virtual time_t getDateTime(int pos)
    throw(SQLException, InvalidArgumentException);
};


class DBEntityStorageStatement : public Statement
{
public:
  virtual ~DBEntityStorageStatement() {}

  virtual void execute()
    throw(SQLException);
  virtual uint32_t executeUpdate()
    throw(SQLException);
  virtual ResultSet* executeQuery()
    throw(SQLException);

  virtual void setString(int pos, const char* str, bool null=false)
    throw(SQLException);

  virtual void setInt8(int pos, int8_t val, bool null=false)
    throw(SQLException);
  virtual void setInt16(int pos, int16_t val, bool null=false)
    throw(SQLException);
  virtual void setInt32(int pos, int32_t val, bool null=false)
    throw(SQLException);
  virtual void setInt64(int pos, int64_t val, bool null=false)
    throw(SQLException);

  virtual void setUint8(int pos, uint8_t val, bool null=false)
    throw(SQLException);
  virtual void setUint16(int pos, uint16_t val, bool null=false)
    throw(SQLException);
  virtual void setUint32(int pos, uint32_t val, bool null=false)
    throw(SQLException);
  virtual void setUint64(int pos, uint64_t val, bool null=false)
    throw(SQLException);

  virtual void setFloat(int pos, float val, bool null=false)
    throw(SQLException);
  virtual void setDouble(int pos, double val, bool null=false)
    throw(SQLException);
  virtual void setLongDouble(int pos, long double val, bool null=false)
    throw(SQLException);

  virtual void setDateTime(int pos, time_t time, bool null=false)
    throw(SQLException);
};

class DDLStatement : public DBEntityStorageStatement
{
public:
  virtual void execute()
    throw(SQLException);
};

#endif
