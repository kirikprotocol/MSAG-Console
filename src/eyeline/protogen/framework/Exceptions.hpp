#ifndef __GENFRAMEWORK_HPP__
#define __GENFRAMEWORK_HPP__

#include <stdexcept>
#include <string>
#include <stdio.h>
#include <inttypes.h>

namespace eyeline{
namespace protogen{
namespace framework{

class BaseException:public std::exception{
public:
  ~BaseException()throw(){}
  const char* what()const throw()
  {
    return msg.c_str();
  }
protected:
  std::string msg;
};

class FieldIsNullException:public BaseException{
public:
  FieldIsNullException(const char* field)
  {
    msg="Attempt to get field ";
    msg+=field;
    msg+" which wasn't set.";
  }
  ~FieldIsNullException()throw(){}
};

class MandatoryFieldMissingException:public BaseException{
public:
  MandatoryFieldMissingException(const char* field)
  {
    msg="Mandatory field ";
    msg+=field;
    msg+" is missing.";
  }
  ~MandatoryFieldMissingException()throw(){}
};

class DuplicateFieldException:public BaseException{
public:
  DuplicateFieldException(const char* field)
  {
    msg="Duplicate field ";
    msg+=field;
    msg+" found.";
  }
  ~DuplicateFieldException()throw(){}
};

class IncompatibleVersionException:public BaseException{
public:
  IncompatibleVersionException(const char* messageName)
  {
    msg="Incompatible versions for message:";
    msg+=messageName;
  }
};

class UnexpectedTag:public BaseException{
public:
  UnexpectedTag(const char* messageName,uint32_t tag)
  {
    char buf[32];
    sprintf(buf,"%u",tag);
    msg="Unexpected tag in message '";
    msg+=messageName;
    msg+="':";
    msg+=buf;
  }
};

class NetworkErrorException:public BaseException{
public:
  NetworkErrorException()
  {
    msg="Network Error";
  }
  ~NetworkErrorException()throw(){}
};

class NetworkInitException:public BaseException{
public:
  NetworkInitException(const char* host,int port)
  {
    char buf[32];
    sprintf(buf,"%d",port);
    msg="Failed to init server at ";
    msg+=host;
    msg+=':';
    msg+=buf;
  }
};

class UnhandledMessage:public BaseException{
public:
  UnhandledMessage(uint32_t tag)
  {
    char buf[32];
    sprintf(buf,"%u",tag);
    msg+="Message with tag ";
    msg+=buf;
    msg+=" not handled.";
  }
};

class ReadBeyonEof:public BaseException{
public:
  ReadBeyonEof()
  {
    msg="Attempt to read data beyond end.";
  }
};

class InvalidValueLength:public BaseException{
public:
  InvalidValueLength(const char* valueType,int len)
  {
    msg="Invalid value length for type ";
    msg+=valueType;
    msg+=":";
    char buf[32];
    sprintf(buf,"%d",len);
    msg+=buf;
  }
};

class WriteToReadonlyBuffer:public BaseException{
public:
  WriteToReadonlyBuffer()
  {
    msg="Attempt to write to readonly buffer.";
  }
};

class InvalidEnumValue:public BaseException{
public:
  InvalidEnumValue(const std::string& name,int64_t value)
  {
    msg="Invalid value of enum ";
    msg+=name;
    msg+=":";
    char buf[32];
    sprintf(buf,"%lld",value);
    msg+=buf;
  }
  InvalidEnumValue(std::string name,const std::string& value)
  {
    msg="Invalid value of enum ";
    msg+=name;
    msg+=":";
    msg+=value;
  }
};

}
}
}

#endif
