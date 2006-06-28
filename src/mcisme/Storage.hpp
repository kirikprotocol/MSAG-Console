//------------------------------------
//  Storage.hpp
//  Routman Michael, 2005-2006
//------------------------------------
//
//	‘айл содержит описание интерфейсного класса Storage.
//

#ifndef ___STORAGE_H
#define ___STORAGE_H

#include <vector>
#include <string>

#include <core/buffers/Array.hpp>
//#include <sms/sms.h>
//#include <sms/sms_const.h>
#include <mcisme/AbntAddr.hpp>
#include <mcisme/DeliveryQueue.hpp>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

namespace smsc { namespace mcisme
{

using std::vector;
using std::string;
//using core::buffers::Array;
//using namespace sms;

//	Register New Event Policy
const uint8_t REJECT_NEW_EVENT = 0x01;
const uint8_t DISPLACE_OLDEST_EVENT = 0x02;

class Storage
{
public:
	Storage(){}
	~Storage(){}
	virtual int Init(smsc::util::config::ConfigView* storageConfig, DeliveryQueue* pDeliveryQueue) = 0;
	virtual int Init(const string& location, time_t _eventLifeTime, uint8_t _maxEvents, DeliveryQueue* pDeliveryQueue) = 0;
	virtual void addEvent(const AbntAddr& CalledNum, const MCEvent& event) = 0;
	virtual bool getEvents(const AbntAddr& CalledNum, vector<MCEvent>& events) = 0;
	virtual void deleteEvents(const AbntAddr& CalledNum, const vector<MCEvent>& events) = 0;
};


};	//  namespace mcisme
};	//  namespace smsc
#endif















//struct	AbntAddrValue
//{
//	union
//	{
//		uint8_t full_addr[17];
//		struct
//		{
//			uint8_t	addr_length:5;
//			uint8_t	num_plan:3
//			uint8_t addr_type;
//			uint8_t addr_sig[15];
//		};
//	};
//};
//
//struct AbntAddr
//{
//  AbntAddrValue value;
//
//  /**
//  * Default конструктор, просто инициализирует некоторые пол€ нул€ми
//  */
//  AbntAddr()
//  {
//    memset((void*)&value, 0x00, sizeof(value));
//  };
//
//  /**
//  *  онструктор дл€ Address, инициализирует пол€ структуры реальными данными.
//  *  опирует даннуе из буфера к себе
//  *
//  * @param _len   длинна буфера _value
//  * @param _type  тип адреса
//  * @param _plan  план нумерации
//  * @param _value значение адреса
//  */
//  AbntAddr(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
//  {
//		value.addr_type = _type;
//		value.num_plan = _plan;
//		setValue(_len, _value);
//  };
//
//  /**
//  *  онструктор копировани€, используетс€ дл€ создани€ адреса по образцу
//  *
//  * @param addr   образец адреса.
//  */
//  AbntAddr(const AbntAddr& addr)
//  {
//    memcpy((void*)&value, (void*)&addr, sizeof(value));
//  };
//
//  virtual ~AbntAddr()
//  {
//    
//  };
//
//  AbntAddr(const char* text)
//  {
//    if(!text || !*text)throw runtime_error("bad address NULL");
//    AddressValue addr_value;
//    int iplan,itype;
//    memset(addr_value,0,sizeof(addr_value));
//    int scaned = sscanf(text,".%d.%d.%20s",
//      &itype,
//      &iplan,
//      addr_value);
//    if ( scaned != 3 )
//    {
//      scaned = sscanf(text,"+%20[0123456789?]s",addr_value);
//      if ( scaned )
//      {
//        iplan = 1;//ISDN
//        itype = 1;//INTERNATIONAL
//      }
//      else
//      {
//        scaned = sscanf(text,"%20[0123456789?]s",addr_value);
//        if ( !scaned )
//          throw runtime_error(string("bad address ")+text);
//        else
//        {
//          iplan = 1;//ISDN
//          itype = 0;//UNKNOWN
//        }
//      }
//    }
//	value.addr_type = (uint8_t)iplan;
//	value.num_plan = (uint8_t)itype;
//	value.addr_length = strlen(addr_value);
//	setValue(strlen(addr_value), addr_value);
//  }
//
//  /**
//  * ѕереопределЄнный оператор '=',
//  * используетс€ дл€ копировани€ адресов друг в друга
//  *
//  * @param addr   ѕрава€ часть оператора '='
//  * @return —сылку на себ€
//  */
//  AbntAddr& operator=(const AbntAddr& addr)
//  {
//	 if(&addr == this)
//		 return (*this);
//	
//	memcpy((void*)&value, (void*)&addr, sizeof(value));
//    return (*this);
//  };
//
//  inline int operator ==(const AbntAddr& addr)const
//  {
//    return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) == 0);
//  };
//
//  inline int operator !=(const AbntAddr& addr)const
//  {
//    return !(*this==addr);
//  };
//
//  bool operator<(const AbntAddr& addr)const
//  {
//    return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) < 0);
//  }
//
//
//  /**
//  * ћетод устанавливает значение адреса и его длинну.
//  * ƒлинна адреса должна быть меньше MAXAbntAddr_VALUE_LENGTH.
//  *
//  * @param _len   длинна нового адреса
//  * @param _value значение нового адреса
//  */
//  inline void setValue(uint8_t _len, const char* _value)
//  {
//    __require__(_len && _value && _value[0]
//	&& _len<sizeof(value.addr_sig));
//
//	memset((void*)&value.addr_sig, 0xFF, sizeof(value.addr_sig));
//	
//	uint8_t	sig1, sig2, i;
//
//	for(i = 0; i < _len>>1; i++)
//	{
//		sig1 = _value[i*2] - 0x30;
//		sig2 = _value[i*2+1] - 0x30;
//		value.addr_sig[i] = (sig2 << 4) | sig1;
//	}
//	if( ((_len>>1)<<1) != _len)
//		value.addr_sig[i] = 0xF0 | (uint8_t)(_value[i*2] - 0x30);
//
//	value.addr_length = _len;
//
//  };
//
//
//  const uint8_t* getAddrSig(void) const
//  {
//	  return (uint8_t*)&(value.full_addr);
//  }
//
//  /**
//  * ћетод копирует значение адреса и возвращает его длинну
//  *
//  * @param _value указатель на буфер куда будет скопированно значение адреса
//  *               буфер должен иметь размер не меньше
//  *               MAXAbntAddr_VALUE_LENGTH+1, чтобы прин€ть любое значение
//  * @return длинна адреса
//  */
//  inline uint8_t getValue(char* _value) const
//  {
//    __require__(_value);
//
//	if (value.addr_length)
//    {
//		for(int i=0; i<value.addr_length; i++)
//			_value[i] = "0123456789"[i%2 ? value.addr_sig[i/2] >> 4 :value.addr_sig[i/2] & 0x0F];
//		_value[value.addr_length] = '\0';
//    }
//	return value.addr_length;
//  }
//
//  /**
//  * ¬озвращает длинну адреса
//  *
//  * @return длинна адреса
//  */
//  inline uint8_t getLength() const
//  {
//	  return value.addr_length;
//  };
//
//  /**
//  * ”станавливает тип адреса
//  *
//  * @param _type  тип адреса
//  */
//  inline void setTypeOfNumber(uint8_t _type)
//  {
//	  value.addr_type = _type;
//  };
//
//  /**
//  * ¬озвращает тип адреса
//  *
//  * @param _type  тип адреса
//  */
//  inline uint8_t getTypeOfNumber() const
//  {
//	  return value.addr_type;
//  };
//
//  /**
//  * ”станавливает план нумерации адреса
//  *
//  * @param _plan  план нумерации адреса
//  */
//  inline void setNumberingPlan(uint8_t _plan)
//  {
//	  value.num_plan = _plan;
//  };
//
//  /**
//  * ¬озвращает план нумерации адреса
//  *
//  * @return план нумерации адреса
//  */
//  inline uint8_t getNumberingPlan() const
//  {
//	  return value.num_plan;
//  };
//
//  inline int getText(char* buf,int buflen)const
//  {
//    char vl[32];
//	getValue(vl);
//	if ( value.addr_type == 1 && value.num_plan == 1 ){
//      return snprintf(buf,buflen,"+%s",vl);
//	}else if (value.addr_type == 0 && value.num_plan == 1){
//      return snprintf(buf,buflen,"%s",vl);
//    }else
//		return snprintf(buf,buflen,".%d.%d.%s",value.addr_type,value.num_plan,vl);
//  }
//  inline int toString(char* buf,int buflen)const{
//    char vl[32];
//	getValue(vl);
//	return snprintf(buf,buflen,".%d.%d.%s",value.addr_type,value.num_plan,vl);
//  }
//  inline std::string toString()const
//  {
//	if(value.addr_length>32)abort();
//    char vl[32];
//    char buf[48];
//	getValue(vl);
//	snprintf(buf,sizeof(buf),".%d.%d.%s",value.addr_type,value.num_plan,vl);
//    return buf;
//  }
//  void Clear()
//  {
//	memset((void*)&value, 0x00, sizeof(value));  }
//
//  };
