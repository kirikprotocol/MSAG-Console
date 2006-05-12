/*
 *  DummyAdapter.cpp
 *  smsc
 *
 *  Created by Serge Lugovoy on 12.05.06.
 *  Copyright 2006 Sibinco. All rights reserved.
 *
 */

#include "DummyAdapter.h"
namespace smsc { namespace util { namespace templates
{

bool DummyGetAdapter::isNull(const char* key)
  throw(AdapterException)
{
    return false;
}

const char* DummyGetAdapter::getString(const char* key)
throw(AdapterException)
{
  return "";
}

int8_t DummyGetAdapter::getInt8(const char* key)
throw(AdapterException)
{
  return 0;
}

int16_t DummyGetAdapter::getInt16(const char* key)
throw(AdapterException)
{
  return 0;
}

int32_t DummyGetAdapter::getInt32(const char* key)
throw(AdapterException)
{
  return 0;
}

int64_t DummyGetAdapter::getInt64(const char* key)
throw(AdapterException)
{
  return 0;
}


uint8_t DummyGetAdapter::getUint8(const char* key)
throw(AdapterException)
{
  return 0;
}

uint16_t DummyGetAdapter::getUint16(const char* key)
throw(AdapterException)
{
  return 0;
}

uint32_t DummyGetAdapter::getUint32(const char* key)
throw(AdapterException)
{
  return 0;
}

uint64_t DummyGetAdapter::getUint64(const char* key)
throw(AdapterException)
{
  return 0;
}


float DummyGetAdapter::getFloat(const char* key)
throw(AdapterException)
{
  return 0;
}

double DummyGetAdapter::getDouble(const char* key)
throw(AdapterException)
{
  return 0;
}

long double DummyGetAdapter::getLongDouble(const char* key)
throw(AdapterException)
{
  return 0;
}


time_t DummyGetAdapter::getDateTime(const char* key)
throw(AdapterException)
{
  return 0;
}

}}}