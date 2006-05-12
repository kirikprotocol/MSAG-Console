#ifndef SMSC_UTIL_TEMPLATES_DUMMYADAPTER
#define SMSC_UTIL_TEMPLATES_DUMMYADAPTER
/*
 *  DummyAdapter.h
 *  smsc
 *
 *  Created by Serge Lugovoy on 12.05.06.
 *  Copyright 2006 Sibinco. All rights reserved.
 *
 */
#include "Adapters.h"

namespace smsc { namespace util { namespace templates
{

class DummyGetAdapter:public GetAdapter{
public:
  
  virtual bool isNull(const char* key) throw(AdapterException);
  virtual const char* getString(const char* key) throw(AdapterException);
  virtual int8_t getInt8(const char* key) throw(AdapterException);
  virtual int16_t getInt16(const char* key) throw(AdapterException);
  virtual int32_t getInt32(const char* key) throw(AdapterException);
  virtual int64_t getInt64(const char* key) throw(AdapterException);
  virtual uint8_t getUint8(const char* key) throw(AdapterException);
  virtual uint16_t getUint16(const char* key) throw(AdapterException);
  virtual uint32_t getUint32(const char* key) throw(AdapterException);
  virtual uint64_t getUint64(const char* key) throw(AdapterException);
  virtual float getFloat(const char* key) throw(AdapterException);
  virtual double getDouble(const char* key) throw(AdapterException);
  virtual long double getLongDouble(const char* key) throw(AdapterException);
  virtual time_t getDateTime(const char* key) throw(AdapterException);
};

}}}
#endif
