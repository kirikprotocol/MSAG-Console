#include "DBEntityStorageStatement.hpp"

bool
DBEntityStorageResultSet::fetchNext() throw(SQLException)
{
  throw SQLException("DBEntityStorageResultSet::fetchNext::: not implemented");
}

bool
DBEntityStorageResultSet::isNull(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::isNull::: not implemented");
}

const char*
DBEntityStorageResultSet::getString(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getString::: not implemented");
}

int8_t
DBEntityStorageResultSet::getInt8(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getInt8::: not implemented");
}

int16_t
DBEntityStorageResultSet::getInt16(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getInt16::: not implemented");
}

int32_t
DBEntityStorageResultSet::getInt32(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getInt32::: not implemented");
}

int64_t
DBEntityStorageResultSet::getInt64(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getInt64::: not implemented");
}

uint8_t
DBEntityStorageResultSet::getUint8(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getUint8::: not implemented");
}

uint16_t
DBEntityStorageResultSet::getUint16(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getUint16::: not implemented");
}

uint32_t
DBEntityStorageResultSet::getUint32(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getUint32::: not implemented");
}

uint64_t
DBEntityStorageResultSet::getUint64(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getUint64::: not implemented");
}

float
DBEntityStorageResultSet::getFloat(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getFloat::: not implemented");
}

double
DBEntityStorageResultSet::getDouble(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getDouble::: not implemented");
}

long double
DBEntityStorageResultSet::getLongDouble(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getLongDouble::: not implemented");
}

time_t
DBEntityStorageResultSet::getDateTime(int pos) throw(SQLException, InvalidArgumentException)
{
  throw SQLException("DBEntityStorageResultSet::getDateTime::: not implemented");
}

void
DBEntityStorageStatement::execute() throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::execute::: not implemented");
}

uint32_t
DBEntityStorageStatement::executeUpdate() throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::executeUpdate::: not implemented");
}

ResultSet*
DBEntityStorageStatement::executeQuery() throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::executeQuery::: not implemented");
}

void
DBEntityStorageStatement::setString(int pos, const char* str, bool null)  throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setString::: not implemented");
}

void
DBEntityStorageStatement::setInt8(int pos, int8_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setInt::: not implemented");
}

void
DBEntityStorageStatement::setInt16(int pos, int16_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setInt16::: not implemented");
}

void
DBEntityStorageStatement::setInt32(int pos, int32_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setInt32::: not implemented");
}

void
DBEntityStorageStatement::setInt64(int pos, int64_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setInt64::: not implemented");
}

void
DBEntityStorageStatement::setUint8(int pos, uint8_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setUint::: not implemented");
}

void
DBEntityStorageStatement::setUint16(int pos, uint16_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setUint16::: not implemented");
}

void
DBEntityStorageStatement::setUint32(int pos, uint32_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setUint32::: not implemented");
}

void
DBEntityStorageStatement::setUint64(int pos, uint64_t val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setUint64::: not implemented");
}

void
DBEntityStorageStatement::setFloat(int pos, float val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setFloat::: not implemented");
}

void
DBEntityStorageStatement::setDouble(int pos, double val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setDouble::: not implemented");
}

void
DBEntityStorageStatement::setLongDouble(int pos, long double val, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setLongDouble::: not implemented");
}

void
DBEntityStorageStatement::setDateTime(int pos, time_t time, bool null) throw(SQLException)
{
  throw SQLException("DBEntityStorageStatement::setDateTime::: not implemented");
}

void
DDLStatement::execute()
  throw(SQLException)
{
  return;
}
