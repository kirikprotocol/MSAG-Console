#undef SMSDEFTAG
#if !defined SMS_IMPLEMENTATION
#define SMSDEFTAG(tag_type,tag,x) static const std::string x
#else
#if defined SMS_IMPLEMENTATION_STATIC
#define SMSDEFTAG(tag_type,tag,x) const std::string Tag::x = #x
#else
#define SMSDEFTAG(tag_type,tag,x) \
  tagToKey[tag].type = tag_type;\
  tagToKey[tag].key = Tag::x;     \
keyToTag[Tag::x]=tag;
#endif
#endif

#undef SMS_BODY_INT_TAG
#undef SMS_BODY_STR_TAG
#undef SMS_BODY_UNKNOWN_TAG
#undef ISMSDEFTAG
#undef SSMSDEFTAG

#define SMS_BODY_INT_TAG 0
#define SMS_BODY_STR_TAG 1
#define SMS_BODY_BIN_TAG 2
#define SMS_BODY_UNKNOWN_TAG 3
#define ISMSDEFTAG(n,x) SMSDEFTAG(SMS_BODY_INT_TAG,n,x);
#define SSMSDEFTAG(n,x) SMSDEFTAG(SMS_BODY_STR_TAG,n,x);
#define BSMSDEFTAG(n,x) SMSDEFTAG(SMS_BODY_BIN_TAG,n,x);

#undef SMS_BODY_TAGS_SET
#define SMS_BODY_TAGS_SET \
  ISMSDEFTAG(0,SMPP_SCHEDULE_DELIVERY_TIME)\
  ISMSDEFTAG(1,SMPP_REPLACE_IF_PRESENT_FLAG)\
  ISMSDEFTAG(2,SMPP_ESM_CLASS)\
  ISMSDEFTAG(3,SMPP_DATA_CODING)\
  ISMSDEFTAG(4,SMPP_SM_LENGTH)\
  ISMSDEFTAG(5,SMPP_REGISTRED_DELIVERY)\
  ISMSDEFTAG(6,SMPP_PROTOCOL_ID)\
  BSMSDEFTAG(7,SMPP_SHORT_MESSAGE)\
  ISMSDEFTAG(8,SMPP_PRIORITY)\
  ISMSDEFTAG(9,SMPP_USER_MESSAGE_REFERENCE)\
  ISMSDEFTAG(10,SMPP_USSD_SERVICE_OP)\
  ISMSDEFTAG(11,SMPP_DEST_ADDR_SUBUNIT)\
  ISMSDEFTAG(12,SMPP_PAYLOAD_TYPE)\
  SSMSDEFTAG(13,SMPP_RECEIPTED_MESSAGE_ID)\
  ISMSDEFTAG(14,SMPP_MS_MSG_WAIT_FACILITIES)\
  ISMSDEFTAG(15,SMPP_USER_RESPONSE_CODE)\
  ISMSDEFTAG(16,SMPP_SAR_MSG_REF_NUM)\
  ISMSDEFTAG(17,SMPP_LANGUAGE_INDICATOR)\
  ISMSDEFTAG(18,SMPP_SAR_TOTAL_SEGMENTS)\
  ISMSDEFTAG(19,SMPP_NUMBER_OF_MESSAGES)\
  BSMSDEFTAG(20,SMPP_MESSAGE_PAYLOAD)\
  ISMSDEFTAG(21,SMPP_DATA_SM) \
  ISMSDEFTAG(23,SMPP_MS_VALIDITY)\
  ISMSDEFTAG(24,SMPP_MSG_STATE) \
  ISMSDEFTAG(25,SMSC_DISCHARGE_TIME) \
  SSMSDEFTAG(26,SMSC_RECIPIENTADDRESS) \
  ISMSDEFTAG(27,SMSC_STATUS_REPORT_REQUEST) \
  ISMSDEFTAG(28,SMSC_USSD_OP)

#define SMS_BODY_TAGS_SET_SIZE 100

#ifndef SMS_DECLARATIONS
#define SMS_DECLARATIONS

/**
* ���� �������� �������� ���������� ��������� ������ ��� ������������� SMS
* � ������� SMS ������. ������������ �������� ��������.
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include <time.h>
#include <string.h>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include <memory>
#include "core/buffers/Hash.hpp"
#include "core/buffers/XHash.hpp"

#include <util/debug.h>

namespace smsc {
  namespace sms  {
    using std::string;
    using smsc::core::buffers::Hash;
    using smsc::core::buffers::XHash;
    using std::runtime_error;
    using std::auto_ptr;

    const int USSD_PSSD_IND   = 0;
    const int USSD_PSSR_IND   = 1;
    const int USSD_USSR_REQ   = 2;
    const int USSD_USSN_REQ   = 3;
    const int USSD_PSSD_RESP  = 16;
    const int USSD_PSSR_RESP  = 17;
    const int USSD_USSR_CONF  = 18;
    const int USSD_USSN_CONF  = 19;

    const int MAX_ESERVICE_TYPE_LENGTH = 5;
    const int MAX_SMESYSID_TYPE_LENGTH = 15;
    const int MAX_ROUTE_ID_TYPE_LENGTH = 20;
    const int MAX_ADDRESS_VALUE_LENGTH = 32;
    const int MAX_SHORT_MESSAGE_LENGTH = 255; // Depricated !!!
    const int MAX_BODY_LENGTH          = 1500;
    const int MAX_FULL_ADDRESS_VALUE_LENGTH = 30;

    typedef uint64_t    SMSId;
    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef char        FullAddressValue[MAX_FULL_ADDRESS_VALUE_LENGTH+1];
    typedef char        SmeSystemIdType[MAX_SMESYSID_TYPE_LENGTH+1];
    typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
    typedef char        RouteId[MAX_ROUTE_ID_TYPE_LENGTH+1];

    /**
    * ��������� Address ������������� ��� ��������
    * ������� � SMS ���������.
    *
    * @author Victor V. Makarov
    * @version 1.0
    * @see SMS
    */
    struct Address
    {
      uint8_t      length, type, plan;
      AddressValue value;

      /**
      * Default �����������, ������ �������������� ��������� ���� ������
      */
      Address() : length(1), type(0), plan(0)
      {
        value[0] = '0'; value[1] = '\0';
      };

      /**
      * ����������� ��� Address, �������������� ���� ��������� ��������� �������.
      * �������� ������ �� ������ � ����
      *
      * @param _len   ������ ������ _value
      * @param _type  ��� ������
      * @param _plan  ���� ���������
      * @param _value �������� ������
      */
      Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
        : length(_len), type(_type), plan(_plan)
      {
        setValue(_len, _value);
      };

      /**
      * ����������� �����������, ������������ ��� �������� ������ �� �������
      *
      * @param addr   ������� ������.
      */
      Address(const Address& addr)
        : length(addr.length), type(addr.type), plan(addr.plan)
      {
        setValue(addr.length, addr.value);
      };

      Address(const char* text)
      {
        AddressValue addr_value;
        int iplan,itype;
        memset(addr_value,0,sizeof(addr_value));
        int scaned = sscanf(text,".%d.%d.%20s",
          &itype,
          &iplan,
          addr_value);
        if ( scaned != 3 )
        {
          scaned = sscanf(text,"+%20[0123456789?]s",addr_value);
          if ( scaned )
          {
            iplan = 1;//ISDN
            itype = 1;//INTERNATIONAL
          }
          else
          {
            scaned = sscanf(text,"%20[0123456789?]s",addr_value);
            if ( !scaned )
              throw runtime_error(string("bad address ")+text);
            else
            {
              iplan = 1;//ISDN
              itype = 0;//UNKNOWN
            }
          }
        }
        plan = (uint8_t)iplan;
        type = (uint8_t)itype;
        length = strlen(addr_value);
        memcpy(value,addr_value,sizeof(addr_value));
      }

      /**
      * ��������������� �������� '=',
      * ������������ ��� ����������� ������� ���� � �����
      *
      * @param addr   ������ ����� ��������� '='
      * @return ������ �� ����
      */
      Address& operator =(const Address& addr)
      {
        type = addr.type; plan = addr.plan;
        setValue(addr.length, addr.value);
        return (*this);
      };

      inline int operator ==(const Address& addr)
      {
        return (length == addr.length &&
                plan == addr.plan && type == addr.type &&
                memcmp(value, addr.value, length) == 0);
      };


      /**
      * ����� ������������� �������� ������ � ��� ������.
      * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
      *
      * @param _len   ������ ������ ������
      * @param _value �������� ������ ������
      */
      inline void setValue(uint8_t _len, const char* _value)
      {
        __require__(_len && _value && _value[0]
          && _len<sizeof(AddressValue));

        memcpy(value, _value, _len*sizeof(uint8_t));
        value[length = _len] = '\0';
      };

      /**
      * ����� �������� �������� ������ � ���������� ��� ������
      *
      * @param _value ��������� �� ����� ���� ����� ������������ �������� ������
      *               ����� ������ ����� ������ �� ������
      *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
      * @return ������ ������
      */
      inline uint8_t getValue(char* _value) const
      {
        __require__(_value);

        if (length)
        {
          memcpy(_value, value, length*sizeof(uint8_t));
          _value[length] = '\0';
        }
        return length;
      }

      // depricated
      inline uint8_t getLenght() const
      {
        return getLength();
      };

      /**
      * ���������� ������ ������
      *
      * @return ������ ������
      */
      inline uint8_t getLength() const
      {
        return length;
      };

      /**
      * ������������� ��� ������
      *
      * @param _type  ��� ������
      */
      inline void setTypeOfNumber(uint8_t _type)
      {
        type = _type;
      };

      /**
      * ���������� ��� ������
      *
      * @param _type  ��� ������
      */
      inline uint8_t getTypeOfNumber() const
      {
        return type;
      };

      /**
      * ������������� ���� ��������� ������
      *
      * @param _plan  ���� ��������� ������
      */
      inline void setNumberingPlan(uint8_t _plan)
      {
        plan = _plan;
      };

      /**
      * ���������� ���� ��������� ������
      *
      * @return ���� ��������� ������
      */
      inline uint8_t getNumberingPlan() const
      {
        return plan;
      };

      inline int getText(char* buf,int buflen)const
      {
        char vl[32];
        memcpy(vl,value,length);
        vl[length]=0;
        if ( type == 1 && plan == 1 ){
          return snprintf(buf,buflen,"+%s",vl);
        }else if (type == 0 && plan == 1){
          return snprintf(buf,buflen,"%s",vl);
        }else
          return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
      }
      inline int toString(char* buf,int buflen)const{
        char vl[32];
        memcpy(vl,value,length);
        vl[length]=0;
        return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
      }
      };

      /**
      * ��������� Descriptor ������������� ��� ��������
      * ���������� ���������������� ����������������� �������
      * (��������, �������� ��� ������� ��������� SMS)
      *
      * @author Victor V. Makarov
      * @version 1.0
      * @see SMS
      */
      struct Descriptor
      {
        uint8_t         mscLength, imsiLength;
        AddressValue    msc, imsi;
        uint32_t        sme;

        /**
        * Default �����������, ������ �������������� ��������� ���� ������
        */
        Descriptor() : mscLength(0), imsiLength(0), sme(0)
        {
          msc[0] = '\0'; imsi[0] = '\0';
        };

        /**
        * ����������� ��� Descriptor, �������������� ���� ��������� ��������� �������.
        * �������� ������ �� ������� � ����
        *
        * @param _mscLen   ������ ������ _msc
        * @param _value �������� ������ MSC
        * @param _imsiLen   ������ ������ _imsi
        * @param _value �������� ������ IMSI
        * @param _sme ����� SME
        */
        Descriptor(uint8_t _mscLen, const char* _msc,
          uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
          : mscLength(_mscLen), imsiLength(_imsiLen), sme(_sme)
        {
          setMsc(mscLength, _msc);
          setImsi(imsiLength, _imsi);
        };

        /**
        * ����������� �����������, ������������ ���
        * �������� ����������� �� �������
        *
        * @param descr ������� �����������.
        */
        Descriptor(const Descriptor& descr)
          : mscLength(descr.mscLength),
          imsiLength(descr.imsiLength), sme(descr.sme)
        {
          setMsc(descr.mscLength, descr.msc);
          setImsi(descr.imsiLength, descr.imsi);
        };

        /**
        * ��������������� �������� '=',
        * ������������ ��� ����������� ������������ ���� � �����
        *
        * @param descr   ������ ����� ��������� '='
        * @return ������ �� ����
        */
        Descriptor& operator =(const Descriptor& descr)
        {
          sme = descr.sme;
          setMsc(descr.mscLength, descr.msc);
          setImsi(descr.imsiLength, descr.imsi);
          return (*this);
        };

        /**
        * ����� ������������� �������� ������ MSC � ��� ������.
        * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
        *
        * @param _len   ������ ������ ������ MSC
        * @param _value �������� ������ ������ MSC
        */
        inline void setMsc(uint8_t _len, const char* _value)
        {
          __require__( _len<sizeof(AddressValue) );

          if (_len && _value)
          {
            memcpy(msc, _value, _len*sizeof(uint8_t));
            msc[mscLength = _len] = '\0';
          }
          else
          {
            memset(msc, 0, sizeof(AddressValue));
            mscLength = 0;
          }
        };

        /**
        * ����� ������������� �������� ������ IMSI � ��� ������.
        * ������ ������ ������ ���� ������ MAX_ADDRESS_VALUE_LENGTH.
        *
        * @param _len   ������ ������ ������ IMSI
        * @param _value �������� ������ ������ IMSI
        */
        inline void setImsi(uint8_t _len, const char* _value)
        {
          __require__( _len<sizeof(AddressValue) );

          if (_len && _value)
          {
            memcpy(imsi, _value, _len*sizeof(uint8_t));
            imsi[imsiLength = _len] = '\0';
          }
          else
          {
            memset(imsi, 0, sizeof(AddressValue));
            imsiLength = 0;
          }
        };

        /**
        * ����� �������� �������� ������ MSC � ���������� ��� ������
        *
        * @param _value ��������� �� ����� ���� ����� ������������ ��������
        *               ������ MSC. ����� ������ ����� ������ �� ������
        *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
        * @return ������ ������ MSC
        */
        inline uint8_t getMsc(char* _value) const
        {
          __require__(_value);

          if (mscLength)
          {
            memcpy(_value, msc, mscLength*sizeof(uint8_t));
            _value[mscLength] = '\0';
          }
          return mscLength;
        }

        /**
        * ����� �������� �������� ������ IMSI � ���������� ��� ������
        *
        * @param _value ��������� �� ����� ���� ����� ������������ ��������
        *               ������ IMSI. ����� ������ ����� ������ �� ������
        *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
        * @return ������ ������ IMSI
        */
        inline uint8_t getImsi(char* _value) const
        {
          __require__(_value);

          if (imsiLength)
          {
            memcpy(_value, imsi, imsiLength*sizeof(uint8_t));
            _value[imsiLength] = '\0';
          }
          return imsiLength;
        }

        // depricated
        inline uint8_t getMscLenght() const
        {
          return getMscLength();
        };

        /**
        * ���������� ������ ������ MSC
        *
        * @return ������ ������ MSC
        */
        inline uint8_t getMscLength() const
        {
          return mscLength;
        };

        // depricated
        inline uint8_t getImsiLenght() const
        {
          return getImsiLength();
        };

        /**
        * ���������� ������ ������ IMSI
        *
        * @return ������ ������ IMSI
        */
        inline uint8_t getImsiLength() const
        {
          return imsiLength;
        };

        /**
        * ������������� ����� SME
        *
        * @return ������ ������ IMSI
        */
        inline void setSmeNumber(uint32_t _sme)
        {
          sme = _sme;
        };

        /**
        * ���������� ����� SME
        *
        * @return ����� SME
        */
        inline uint32_t getSmeNumber() const
        {
          return sme;
        };
      };


      class SMSDict
      {
      public:
        virtual void setStrProperty(const string& s,const string& value)= 0;
        virtual void setIntProperty(const string& s,const uint32_t value) = 0;
        virtual void setBinProperty(const string& s,const char* data, unsigned len) = 0;
        virtual string getStrProperty(const string& s) = 0;
        virtual uint32_t getIntProperty(const string& s) = 0;
        virtual const char* getBinProperty(const string& s,unsigned* len) = 0;
        virtual bool hasIntProperty(const string& s) = 0;
        virtual bool hasStrProperty(const string& s) = 0;
        virtual bool hasBinProperty(const string& s) = 0;
        virtual ~SMSDict(){}
      };

      struct StringHashFunc{
        static inline unsigned int CalcHash(const string& key)
        {
          const unsigned char* curr = (const unsigned char*)key.c_str();
          unsigned count = *curr;
          while(*curr)
          {
            count += 37 * count + *curr;
            curr++;
          }
          count=(unsigned)(( ( count * (unsigned)19L ) + (unsigned)12451L ) % (unsigned)8882693L);
          return count;
        }
      };

      class Tag
      {
      public:
        SMS_BODY_TAGS_SET
      };

      class TagHash
      {
        struct SMSTag
        {
          string key;
          int type;
          SMSTag():type(SMS_BODY_UNKNOWN_TAG){}
        };
        XHash<string,int,StringHashFunc> keyToTag;
        SMSTag* tagToKey;
        public:
          TagHash();
          ~TagHash();
          string* getStrKeyForString(int tag)
          {
            throw_if(tag >= SMS_BODY_TAGS_SET_SIZE);
            if ( tagToKey[tag].type == SMS_BODY_STR_TAG )
              return &tagToKey[tag].key;
            else
            /*{
            char buff[128];
            snprintf(buff,sizeof(buff),"tag %d is not STRING PRPERTY tag",tag);
            throw runtime_error(buff);
          }*/ return 0;
          }
          string* getStrKeyForBin(int tag)
          {
            throw_if(tag >= SMS_BODY_TAGS_SET_SIZE);
            if ( tagToKey[tag].type == SMS_BODY_BIN_TAG )
              return &tagToKey[tag].key;
            else
            /*{
            char buff[128];
            snprintf(buff,sizeof(buff),"tag %d is not STRING PRPERTY tag",tag);
            throw runtime_error(buff);
          }*/ return 0;
          }
          string* getStrKeyForInt(int tag)
          {
            throw_if(tag >= SMS_BODY_TAGS_SET_SIZE);
            if ( tagToKey[tag].type == SMS_BODY_INT_TAG )
              return &tagToKey[tag].key;
            else
            /*{
            char buff[128];
            snprintf(buff,sizeof(buff),"tag %d is not INT PRPERTY tag",tag);
            throw runtime_error(buff);
          }*/ return 0;
          }
          int getTag(const string& key)
          {
            const int* tag = keyToTag.GetPtr(key.c_str());
            if ( !tag )
              throw runtime_error(string("key ")+key+" is invalid for tagging");
            throw_if(*tag >= SMS_BODY_TAGS_SET_SIZE);
            return *tag;
          }
          int getTag(const char* key)
          {
            const int* tag = keyToTag.GetPtr(key);
            if ( !tag )
              throw runtime_error(string("key ")+key+" is invalid for tagging");
            throw_if(*tag >= SMS_BODY_TAGS_SET_SIZE);
            return *tag;
          }
      };

      extern TagHash tag_hash;


      class TemporaryBodyStr
      {
        XHash<string,string,StringHashFunc> hash;
      public:
        int getRequiredBufferSize()
        {
          int size = 0;
          string key;
          string* value;
          hash.First();
          while(hash.Next(key,value))
          {
            __require__(value!=0);
            size+=value->length()+4+1+2;
          }
//          __trace2__("TemporaryBodyStr size: %d",size);
          return size;
        }

        int hasValue(const string& key)
        {
          return hash.Exists(key);
        }

        string getValue(const string& key)
        {
          const string* value = hash.GetPtr(key);
          if ( !value ) return "";
          else return string(*value);
        }

        void setValue(const string& key,const string& value)
        {
          hash[key] = value;
        }

        int encode(uint8_t* buffer,int offs,int length)
        {
          string key;
          string* value;
          hash.First();
          while(hash.Next(key,value))
          {
            __require__(value!=0);
            __require__(offs+4<length);
            uint32_t len = (uint32_t)value->length()+1;
            uint16_t tag = tag_hash.getTag(key);
            {
              uint16_t tmp16 = htons(tag);;
              //*(uint16_t*)(buffer+offs) = htons(tag);
              memcpy(buffer+offs,&tmp16,2);
              uint32_t tmp32 = htonl(len);
              //*(uint32_t*)(buffer+offs+2) = htonl(len);
              memcpy(buffer+offs+2,&tmp32,4);
            }
            offs+=4+2;
//            __trace2__("Senc: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key:"NULL",len,offs,length);
            __require__(offs+len<=(unsigned)length);
            memcpy(buffer+offs,value->c_str(),len);
            offs+=len;
          }
          return offs;
        }

        void decode(uint8_t* buffer, int length)
        {
          hash.Empty();
          if ( !buffer || !length ) return;
          for(int pos = 0; pos+4 < length;)
          {
            uint16_t tag;
            memcpy(&tag,buffer+pos,2);
            tag = ntohs(tag);
            uint32_t len;
            memcpy(&len,buffer+pos+2,4);
            len = ntohl(len);
            pos+=4+2;
            __require__(pos+len<=(unsigned)length);
            string* key = tag_hash.getStrKeyForString(tag);
//            __trace2__("Sdec: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key->c_str():"NULL",len,pos,length);
            if ( key )
            {
              __require__(*(buffer+pos+len-1) == 0);
              hash[*key] = string((char*)buffer+pos);
            }
            pos+=len;
          }
        }
      };

      class TemporaryBodyInt
      {
        XHash<string,int,StringHashFunc> hash;
      public:
        int getRequiredBufferSize()
        {
          int size = 0;
          string key;
          int* value;
          hash.First();
          while(hash.Next(key,value))
          {
            //__reqruire__(value!=0);
            //size+=value->length();
            size+=4+4+2;
          }
//          __trace2__("TemporaryBodyInt size: %d",size);
          return size;
        }

        bool hasValue(const string& key)
        {
          return hash.Exists(key)!=0;
        }

        int getValue(const string& key)
        {
          const int* value = hash.GetPtr(key);
          if ( !value ) return 0;
          else return *value;
        }

        void setValue(const string& key,int value)
        {
          hash[key] = value;
        }

        int encode(uint8_t* buffer,int offs,int length)
        {
          string key;
          int* value;
          hash.First();
          while(hash.Next(key,value))
          {
            __require__(value!=0);
            __require__(offs+4<length);
            uint32_t len = 4;
            uint16_t tag = tag_hash.getTag(key);
            {
              uint16_t temp16 = htons(tag);
              memcpy(buffer+offs,&temp16,2);
              uint32_t temp32 = htonl(4);
              memcpy(buffer+offs+2,&temp32,4);
            }
            offs+=4+2;
//            __trace2__("Ienc: tag=%hd key=%s len=%hd pos=%d length=%d val=%d",tag,key?key:"NULL",len,offs,length,*value);
            __require__(offs+len<=(unsigned)length);
            //memcpy(buffer+pos,value->c_str(),len);
            {
              uint32_t temp32 = htonl(*value);
              memcpy(buffer+offs,&temp32,4);
            }
            offs+=4;
          }
          return offs;
        }

        void decode(uint8_t* buffer, int length)
        {
          hash.Empty();
          if ( !buffer || !length ) return;
          for(int pos = 0; pos+4 < length;)
          {
            uint16_t tag;
            memcpy(&tag,buffer+pos,2);
            tag = ntohs(tag);
            uint32_t len;
            memcpy(&len,buffer+pos+2,4);
            len = ntohl(len);
            pos+=4+2;
            __require__(pos+len<=(unsigned)length);
            string* key = tag_hash.getStrKeyForInt(tag);
            if ( key )
            {
              __require__(len == 4);
              uint32_t tmp;
              memcpy(&tmp,buffer+pos,4);
              hash[*key] = ntohl(tmp);
//              __trace2__("Idec: tag=%hd key=%s len=%hd pos=%d length=%d val=%d",tag,key?key->c_str():"NULL",len,pos,length,ntohl(tmp));
            }
            pos+=len;
          }
        }
      };

      class TemporaryBodyBin
      {
        class COStr{
          auto_ptr<char> data;
          unsigned len;
        public:
          unsigned length() {return len;}
          void set(const char* text,unsigned len){
            this->len = len;
            data = auto_ptr<char>(new char[len]);
            memcpy(data.get(),text,len);
          }
          const char* dat() { return data.get();}
          const char* get(unsigned* _len) const {
            require(_len!=0);
            *_len = this->len;
            return data.get();
          }
          COStr& operator = (const COStr& costr){
            unsigned l;
            const char* t = costr.get(&l);
            set(t,l);
            return *this;
          }
          COStr() : len(0) {}
          COStr(const COStr& costr): len(0){operator =(costr);}
        };
        XHash<string,COStr,StringHashFunc> hash;
      public:
        int getRequiredBufferSize()
        {
          int size = 0;
          string key;
          COStr* value;
          hash.First();
          while(hash.Next(key,value))
          {
            __require__(value!=0);
            size+=value->length()+4+2;
          }
//          __trace2__("TemporaryBodyBin size: %d",size);
          return size;
        }

        int hasValue(const string& key)
        {
          return hash.Exists(key);
        }

        const char* getValue(const string& key,unsigned* len)
        {
          const COStr* value = hash.GetPtr(key);
          if ( !value ) return 0;
          else return value->get(len);
        }

        void setValue(const string& key, const char* value, unsigned len)
        {
          hash[key].set(value,len);
        }

        int encode(uint8_t* buffer,int offs,int length)
        {
          string key;
          COStr* value;
          hash.First();
          while(hash.Next(key,value))
          {
            __require__(value!=0);
            __require__(offs+4<length);
            uint32_t len = (uint32_t)value->length();
            uint16_t tag = tag_hash.getTag(key);
            {
              uint16_t tmp16 = htons(tag);;
              //*(uint16_t*)(buffer+offs) = htons(tag);
              memcpy(buffer+offs,&tmp16,2);
              uint32_t tmp32 = htonl(len);
              //*(uint32_t*)(buffer+offs+2) = htonl(len);
              memcpy(buffer+offs+2,&tmp32,4);
            }
            offs+=4+2;
//            __trace2__("Benc: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key:"NULL",len,offs,length);
            __require__(offs+len<=(unsigned)length);
            memcpy(buffer+offs,value->dat(),len);
            offs+=len;
          }
          return offs;
        }

        void decode(uint8_t* buffer, int length)
        {
          hash.Empty();
          if ( !buffer || !length ) return;
          for(int pos = 0; pos+4 < length;)
          {
            uint16_t tag;
            memcpy(&tag,buffer+pos,2);
            tag = ntohs(tag);
            uint32_t len;
            memcpy(&len,buffer+pos+2,4);
            len = ntohl(len);
            pos+=4+2;
            __require__(pos+len<=(unsigned)length);
            string* key = tag_hash.getStrKeyForBin(tag);
//            __trace2__("Bdec: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key->c_str():"NULL",len,pos,length);
            if ( key )
            {
              //__require__(*(buffer+pos+len) == 0);
              hash[*key].set((char*)buffer+pos,len);
            }
            pos+=len;
          }
        }
      };

      /**
      * ��������� ��������� ���� ���������
      *
      * @author Victor V. Makarov
      * @version 1.0
      * @see SMS
      */
      struct Body
      {
      private:
        mutable auto_ptr<uint8_t> buff;
        mutable int         buffLen;
        mutable TemporaryBodyStr temporaryBodyStr;
        mutable TemporaryBodyInt temporaryBodyInt;
        mutable TemporaryBodyBin temporaryBodyBin;
      public:
        /**
        * Default �����������, ������ �������������� ���� ������
        */
        Body() : buffLen(0) {};

        /**
        * ����������� ��� Body, �������������� ���� ��������� ��������� �������.
        * �������� ������ �� ������ � ����
        *
        * @param data   ������ � ���� ���������
        * @param len    ������ ������ data
        */
        Body(uint8_t* data, int len)
          : buff(0), buffLen(0)
        {
          setBuffer(data, len);
        };

        /**
        * ����������� �����������, ������������ ��� �������� ���� �� �������
        *
        * @param body   ������� ����.
        */
        Body(const Body& body)
          : buffLen(0)
        {
          /*int len = body.getBufferLength();
          uint8_t* b = new uint8_t[len];
          memcpy(b,body.getBuffer(),len);
          setBuffer(b, len);*/
          buff=auto_ptr<uint8_t>(0);
          temporaryBodyStr=body.temporaryBodyStr;
          temporaryBodyInt=body.temporaryBodyInt;
          temporaryBodyBin=body.temporaryBodyBin;
        };

        /**
        * ��������������� �������� '=',
        * ������������ ��� ����������� ��� ��������� ���� � �����
        *
        * @param body   ������ ����� ��������� '='
        * @return ������ �� ����
        */
        Body& operator =(const Body& body)
        {
          buffLen=0;
          buff=auto_ptr<uint8_t>(0);
          temporaryBodyStr=body.temporaryBodyStr;
          temporaryBodyInt=body.temporaryBodyInt;
          temporaryBodyBin=body.temporaryBodyBin;
          /*int len = body.getBufferLength();
          uint8_t* b = new uint8_t[len];
          memcpy(b,body.getBuffer(),len);
          setBuffer(b, len);*/
          return (*this);
        };

        /**
        * ����� ���������� ���� ���������.
        * �������� ������, ����� ������������� � ���������� ����������.
        * ���� �� �����������������
        *
        * @return �������������� ���� ���������. ������ NULL.
        */
        char* getDecodedText()
        {
          return 0L;
        };

        uint8_t* getBuffer()    const
        {
          int blength = getRequiredBufferSize();
          if ( !buff.get() || buffLen < blength )
          {
            buff = auto_ptr<uint8_t>(new uint8_t[blength]);
          }
          buffLen = blength;
          encode(buff.get(),buffLen);
          return buff.get();
        }

        int getBufferLength() const
        {
          return getRequiredBufferSize();
        }

        void setBuffer(uint8_t* buffer, int length)
        {
          //encode(buffer,length);
          decode(buffer,length);
          this->buff = auto_ptr<uint8_t>(buffer);
          //delete buffer;
        }

        unsigned getShortMessageLength()
        {
            // Need to implement this functionality.
            return getIntProperty(Tag::SMPP_SM_LENGTH);
        }

        void encode(uint8_t* buffer,int& length) const
        {
          __require__( buffer != 0 );
          __require__( length >= 0 );
          __require__( length >= getRequiredBufferSize() );
          int offs;
          offs = temporaryBodyStr.encode(buffer,0,length);
          offs = temporaryBodyInt.encode(buffer,offs,length);
          offs = temporaryBodyBin.encode(buffer,offs,length);
        };

        int getRequiredBufferSize() const
        {
          int size = temporaryBodyStr.getRequiredBufferSize()+
            temporaryBodyInt.getRequiredBufferSize()+
            temporaryBodyBin.getRequiredBufferSize();
//          __trace2__("RequiredBufferSize: %d",size);
          return size;
        };

        void decode(uint8_t* buffer,int length)
        {
          //__require__( buffer != 0 );
          __require__( length >= 0 );
          temporaryBodyStr.decode(buffer,length);
          temporaryBodyInt.decode(buffer,length);
          temporaryBodyBin.decode(buffer,length);
        };

        void setStrProperty(const string& s,const string& value)
        {
          temporaryBodyStr.setValue(s,value);
        }

        void setBinProperty(const string& s,const char* value, unsigned len)
        {
          temporaryBodyBin.setValue(s,value,len);
        }

        void setIntProperty(const string& s,const uint32_t value)
        {
          temporaryBodyInt.setValue(s,value);
        }

        string getStrProperty(const string& s)
        {
          return temporaryBodyStr.getValue(s);
        }

        const char* getBinProperty(const string& s,unsigned* len)
        {
          return temporaryBodyBin.getValue(s,len);
        }

        uint32_t getIntProperty(const string& s)
        {
          return temporaryBodyInt.getValue(s);
        }

        bool hasIntProperty(const string& s)
        {
          return temporaryBodyInt.hasValue(s);
        }

        bool hasStrProperty(const string& s)
        {
          return temporaryBodyStr.hasValue(s);
        }

        bool hasBinProperty(const string& s)
        {
          return temporaryBodyBin.hasValue(s);
        }
      };

      const uint8_t SMSC_BYTE_ENROUTE_STATE       = (uint8_t)0;
      const uint8_t SMSC_BYTE_DELIVERED_STATE     = (uint8_t)1;
      const uint8_t SMSC_BYTE_EXPIRED_STATE       = (uint8_t)2;
      const uint8_t SMSC_BYTE_UNDELIVERABLE_STATE = (uint8_t)3;
      const uint8_t SMSC_BYTE_DELETED_STATE       = (uint8_t)4;

      /**
      * ��������� ��������� SMS � ��������� SMS ������
      *
      * @see SMS
      */
      typedef enum {
        ENROUTE         = SMSC_BYTE_ENROUTE_STATE,
        DELIVERED       = SMSC_BYTE_DELIVERED_STATE,
        EXPIRED         = SMSC_BYTE_EXPIRED_STATE,
        UNDELIVERABLE   = SMSC_BYTE_UNDELIVERABLE_STATE,
        DELETED         = SMSC_BYTE_DELETED_STATE
      } State;

      /**
      * ��������� ����������� SMS � ��������� ��������
      * SMS �����. ������������ �������� ��������.
      *
      * @author Victor V. Makarov
      * @version 1.0
      * @see MessageStore
      */
      struct SMS: public SMSDict
      {
        State       state;
        time_t      submitTime;     // �����/���� ����������� �� SMSC
        time_t      validTime;      // �����/���� �� �������� ��������� �������

        uint32_t    attempts;       // ���������� ���������� ������� ��������
        uint32_t    lastResult;     // ��������� ��������� �������
        time_t      lastTime;       // �����/���� ��������� ������� ��������
        time_t      nextTime;       // �����/���� ��������� ������� ��������

        Address     originatingAddress;
        Address     destinationAddress;
        Address     dealiasedDestinationAddress;

        uint16_t    messageReference;
        EService    eServiceType;

        bool        needArchivate;
        uint8_t     deliveryReport;
        uint8_t     billingRecord;

        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;

        Body        messageBody;    // ���� ��������� + PDU ����.
        bool        attach;

        RouteId     routeId;        // Additional fields
        int32_t     serviceId;      // Additional fields
        int32_t     priority;       // Additional fields

        SmeSystemIdType srcSmeId;
        SmeSystemIdType dstSmeId;

        /**
        * Default �����������, ������ �������������� ���� state ��� ENROUTE
        * � ������ ���� ���������� ����������
        */
        SMS() : state(ENROUTE), submitTime(0), validTime(0),
          attempts(0), lastResult(0), lastTime(0), nextTime(0),
          messageReference(0), needArchivate(true),
          deliveryReport(0), billingRecord(0), attach(false),
          serviceId(0), priority(0)
        {
          eServiceType[0]='\0'; routeId[0]='\0';
          srcSmeId[0]='\0'; dstSmeId[0]='\0';
        };


        /**
        * ����������� �����������, ������������ ��� ��������
        * SMS �� �������
        *
        * @param sms    ������� SMS
        */
        SMS(const SMS& sms) :
          state(sms.state),
          submitTime(sms.submitTime),
          validTime(sms.validTime),
          attempts(sms.attempts),
          lastResult(sms.lastResult),
          lastTime(sms.lastTime),
          nextTime(sms.nextTime),
          originatingAddress(sms.originatingAddress),
          destinationAddress(sms.destinationAddress),
          dealiasedDestinationAddress(sms.dealiasedDestinationAddress),
          messageReference(sms.messageReference),
          needArchivate(sms.needArchivate),
          deliveryReport(sms.deliveryReport),
          billingRecord(sms.billingRecord),
          originatingDescriptor(sms.originatingDescriptor),
          destinationDescriptor(sms.destinationDescriptor),
          messageBody(sms.messageBody),
          attach(sms.attach),
          serviceId(sms.serviceId),
          priority(sms.priority)
        {
          strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
          strncpy(routeId, sms.routeId, sizeof(routeId));
          strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
          strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));
        };

        /**
        * ��������������� �������� '=',
        * ������������ ��� ����������� ���������
        *
        * @param sms   ������ ����� ��������� '='
        * @return ������ �� ����
        */
        SMS& operator =(const SMS& sms)
        {
          state = sms.state;
          submitTime = sms.submitTime;
          validTime = sms.validTime;
          attempts = sms.attempts;
          lastResult = sms.lastResult;
          lastTime = sms.lastTime;
          nextTime = sms.nextTime;
          originatingAddress = sms.originatingAddress;
          destinationAddress = sms.destinationAddress;
          dealiasedDestinationAddress = sms.dealiasedDestinationAddress;
          messageReference = sms.messageReference;
          needArchivate = sms.needArchivate;
          deliveryReport = sms.deliveryReport;
          billingRecord = sms.billingRecord;
          originatingDescriptor = sms.originatingDescriptor;
          destinationDescriptor = sms.destinationDescriptor;
          messageBody = sms.messageBody;
          attach = sms.attach;
          serviceId = sms.serviceId;
          priority = sms.priority;

          strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
          strncpy(routeId, sms.routeId, sizeof(routeId));
          strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
          strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));

          return (*this);
        };

        /**
        * ���������� ��������� ���������
        *
        * @return ��������� ���������
        */
        inline State getState() const
        {
          return state;
        };

        /**
        * ������������� ����� �����������.
        * �������� ����� �� ���������� ���������
        *
        * @param length ������ ������ (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
        * @param type   ��� ������
        * @param plan   ���� ��������� ������
        * @param buff   �������� ������
        * @see Address
        */
        inline void setOriginatingAddress(uint8_t length, uint8_t type,
          uint8_t plan, const char* buff)
        { // Copies address value from 'buff' to static structure
          originatingAddress.setTypeOfNumber(type);
          originatingAddress.setNumberingPlan(plan);
          originatingAddress.setValue(length, buff);
        };

        /**
        * ������������� ����� �����������
        * �������� ����� �� ���������� ���������
        *
        * @param address ����� ����� �����������
        * @see Address
        */
        inline void setOriginatingAddress(const Address& address)
        { // Copies address value from 'address' to static structure
          originatingAddress = address;
        };

        /**
        * ���������� ����� �����������
        *
        * @return ����� �����������
        * @see Address
        */
        inline const Address& getOriginatingAddress() const
        {
          return originatingAddress;
        };

        /**
        * ���������� ����� �����������
        *
        * @return ����� �����������
        * @see Address
        */
        inline Address& getOriginatingAddress()
        {
          return originatingAddress;
        };

        /**
        * ������������� ����� ����������
        * �������� ����� �� ���������� ���������
        *
        * @param length ������ ������ (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
        * @param type   ��� ������
        * @param plan   ���� ��������� ������
        * @param buff   �������� ������
        * @see Address
        */
        inline void setDestinationAddress(uint8_t length, uint8_t type,
          uint8_t plan, const char* buff)
        { // Copies address value from 'buff' to static structure
          destinationAddress.setTypeOfNumber(type);
          destinationAddress.setNumberingPlan(plan);
          destinationAddress.setValue(length, buff);
        };

        /**
        * ������������� ����� ����������
        * �������� ����� �� ���������� ���������
        *
        * @param address ����� ����� ����������
        * @see Address
        */
        inline void setDestinationAddress(const Address& address)
        { // Copies address value from 'address' to static structure
          destinationAddress = address;
        };

        /**
        * ���������� ����� ����������
        *
        * @return ����� ����������
        * @see Address
        */
        inline const Address& getDestinationAddress() const
        {
          return destinationAddress;
        };

        /**
        * ���������� ����� ����������
        *
        * @return ����� ����������
        * @see Address
        */
        inline Address& getDestinationAddress()
        {
          return destinationAddress;
        };

        inline void setDealiasedDestinationAddress(uint8_t length, uint8_t type,
          uint8_t plan, const char* buff)
        { // Copies address value from 'buff' to static structure
          dealiasedDestinationAddress.setTypeOfNumber(type);
          dealiasedDestinationAddress.setNumberingPlan(plan);
          dealiasedDestinationAddress.setValue(length, buff);
        };

        inline void setDealiasedDestinationAddress(const Address& address)
        { // Copies address value from 'address' to static structure
          dealiasedDestinationAddress = address;
        };

        inline const Address& getDealiasedDestinationAddress() const
        {
          return dealiasedDestinationAddress;
        };

        inline Address& getDealiasedDestinationAddress()
        {
          return dealiasedDestinationAddress;
        };

        /**
        * ������������� ���������� �����������
        * �������� ����� �� ���������� ���������
        *
        * @param descriptor ����� ���������� �����������
        * @see Descriptor
        */
        inline void setOriginatingDescriptor(const Descriptor& descriptor)
        { // Copies descriptor from 'descriptor' to static structure
          originatingDescriptor = descriptor;
        };

        /**
        * ������������� ���������� �����������
        * �������� ����� �� ���������� ���������
        *
        * @param _mscLen   ������ ������ _msc
        * @param _value �������� ������ MSC
        * @param _imsiLen   ������ ������ _imsi
        * @param _value �������� ������ IMSI
        * @param _sme ����� SME
        *
        * @see Descriptor
        */
        inline void setOriginatingDescriptor(uint8_t _mscLen, const char* _msc,
          uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
        { // Copies descriptor from 'descriptor' to static structure
          originatingDescriptor.setMsc(_mscLen, _msc);
          originatingDescriptor.setImsi(_imsiLen, _imsi);
          originatingDescriptor.setSmeNumber(_sme);
        };

        /**
        * ���������� ���������� �����������
        *
        * @return ���������� �����������
        * @see Descriptor
        */
        inline const Descriptor& getOriginatingDescriptor() const
        {
          return originatingDescriptor;
        };

        /**
        * ���������� ���������� �����������
        *
        * @return ���������� �����������
        * @see Descriptor
        */
        inline Descriptor& getOriginatingDescriptor()
        {
          return originatingDescriptor;
        };

        /**
        * ������������� ���������� ����������
        * �������� ����� �� ���������� ���������
        *
        * @param descriptor ����� ���������� ����������
        * @see Descriptor
        *
        inline void setDestinationDescriptor(const Descriptor& descriptor)
        { // Copies descriptor from 'descriptor' to static structure
            destinationDescriptor = descriptor;
        };*/

        /**
        * ���������� ���������� ����������
        *
        * @return ���������� ����������
        * @see Descriptor
        */
        inline const Descriptor& getDestinationDescriptor() const
        {
          return destinationDescriptor;
        };

        /**
        * ���������� ���������� ����������
        *
        * @return ���������� ����������
        * @see Descriptor
        *
        inline Descriptor& getDestinationDescriptor()
        {
        return destinationDescriptor;
          };*/


        /**
        * ������������� ����� ���������� ���������.
        *
        * @param time   ����, �� ������� ��������� �������
        *               (�� �������� �������).
        */
        inline void setValidTime(time_t time)
        {
          validTime = time;
        };

        /**
        * ���������� ����� ���������� ���������.
        *
        * @return time   ����, �� ������� ��������� �������
        *         (�� �������� �������).
        */
        inline time_t getValidTime() const
        {
          return validTime;
        };

        /**
        * ������������� ����� ����������� ��������� � SMSC
        *
        * @param time   ����� ����������� ��������� � SMSC
        */
        inline void setSubmitTime(time_t time)
        {
          submitTime = time;
        };

        /**
        * ���������� ����� ����������� ��������� � SMSC
        *
        * @return ����� ����������� ��������� � SMSC
        */
        inline time_t getSubmitTime() const
        {
          return submitTime;
        };

        /**
        * ������������� ����� ��������� ������� �������� ��������� �� SMSC
        *
        * @param time   ����� ��������� ������� �������� ��������� �� SMSC
        *
        inline void setLastTime(time_t time)
        {
            lastTime = time;
        };*/

        /**
        * ���������� ����� ��������� ������� �������� ��������� �� SMSC
        *
        * @return ����� ��������� ������� �������� ��������� �� SMSC
        */
        inline time_t getLastTime() const
        {
          return lastTime;
        };

        /**
        * ������������� ����� ��������� ������� �������� ��������� �� SMSC
        *
        * @param time   ����� ��������� ������� �������� ��������� �� SMSC
        */
        inline void setNextTime(time_t time)
        {
          nextTime = time;
        };

        /**
        * ���������� ����� ��������� ������� �������� ��������� �� SMSC
        *
        * @return ����� ��������� ������� �������� ��������� �� SMSC
        */
        inline time_t getNextTime() const
        {
          return nextTime;
        };

        /**
        * ������������� ����� ��������� (MR),
        * ��� ������������� ��������� ���������� � ������ ������
        *
        * @param mr     ����������������� ����� ��������� (MR)
        */
        inline void setMessageReference(uint16_t mr)
        {
          messageReference = mr;
        };

        /**
        * ���������� ����� ��������� (MR),
        * ��� ������������� ��������� ���������� � ������ ������
        *
        * @return ����������������� ����� ��������� (MR)
        */
        inline uint16_t getMessageReference() const
        {
          return messageReference;
        };

        /**
        * ������������� ��� ������ � �������� ���������
        *
        * @param req    ��� ������ � �������� ���������
        */
        inline void setDeliveryReport(uint8_t report)
        {
          deliveryReport = report;
        };

        /**
        * ���������� ��� ������ � �������� ���������
        *
        * @return      ��� ������ � �������� ���������
        */
        inline uint8_t getDeliveryReport() const
        {
          return deliveryReport;
        };

        /**
        * ������������� ��� �������� ���������
        *
        * @param req    ��� �������� ���������
        */
        inline void setBillingRecord(uint8_t billing)
        {
          billingRecord = billing;
        };

        /**
        * ���������� ��� �������� ���������
        *
        * @return      ��� �������� ���������
        */
        inline uint8_t getBillingRecord() const
        {
          return billingRecord;
        };

        /**
        * ������������� �������, ����� �� ������������ ��������� �����
        * �������� ��� ���������� ������� ��������
        *
        * @param arc    �������, ����� �� ������������ ���������
        */
        inline void setArchivationRequested(bool arc)
        {
          needArchivate = arc;
        };

        /**
        * ���������� �������, ����� �� ������������ ��������� �����
        * �������� ��� ���������� ������� ��������
        *
        * @return �������, ����� �� ����� �� ������������ ��������� (�� / ���)
        */
        inline bool isArchivationRequested() const
        {
          return needArchivate;
        };

        /**
        * ���������� ������� � ������
        * ������/��������������/���������� ��������� ������� �������� ���������
        *
        * @return ������� ������/��������������/���������� ���������
        */
        inline uint32_t getLastResult() const
        {
          return lastResult;
        };

        /**
        * ������������� ���������� ������ ��� �������� ���������
        *
        * @param count  ���������� ������ ��� �������� ���������
        *
        inline void setAttemptsCount(uint32_t count)
        {
        attempts = count;
        };*/

        /**
        * ���������� ���������� ������ ��� �������� ���������
        *
        * @return ���������� ������ ��� �������� ���������
        */
        inline uint32_t getAttemptsCount() const
        {
          return attempts;
        };

        /**
        * ���������� ���� ���������
        *
        * @return ���� ���������
        * @see Body
        */
        inline const Body& getMessageBody() const
        {
          return messageBody;
        };

        /**
        * ���������� ���� ���������
        *
        * @return ���� ���������
        * @see Body
        */
        inline Body& getMessageBody()
        {
          return messageBody;
        };

        inline void setStringField(char* field, const char* val, int maxLen)
        {
            if (val) {
              strncpy(field, val, maxLen); field[maxLen]='\0';
            } else field[0]='\0';
        }
        inline void getStringField(const char* field, char* val, int maxLen) const
        {
            __require__(val);
            val[0]='\0'; strncpy(val, field, maxLen); val[maxLen]='\0';
        }

        /**
        * ����� ������������� ���-��� ������� SME.
        *
        * @param type ���-��� SME
        */
        inline void setEServiceType(const char* type)
        {
            setStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ���-��� �������
        *
        * @param type   ��������� �� ����� ���� ����� ������������ ���
        *               ����� ������ ����� ������ �� ������
        *               MAX_ESERVICE_TYPE_LENGTH+1, ����� �������
        *               ����� ��������
        */
        inline void getEServiceType(char* type) const
        {
            getStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ���-��� �������
        *
        * @return ���-��� �������
        */
        inline const char* getEServiceType() const
        {
            return eServiceType;
        };

        /**
        *
        * ������������� ��������� ���������.
        *
        * @param pri    ��������� ���������
        */
        inline void setPriority(int32_t pri) {
            priority = pri;
        };
        /**
        *
        * ���������� ��������� ���������.
        *
        * @return ��������� ���������
        */
        inline int32_t getPriority() const {
            return priority;
        };

        /**
        *
        * ������������� ������������� �������.
        *
        * @param id    ������������� �������
        */
        inline void setServiceId(int32_t id) {
            serviceId = id;
        };
        /**
        *
        * ���������� ������������� �������.
        *
        * @return ������������� �������
        */
        inline uint32_t getServiceId() const {
            return serviceId;
        };

        /**
        * ����� ������������� ������������� ��������.
        *
        * @param route ������������� ��������
        */
        inline void setRouteId(const char* route)
        {
            setStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ������������� ��������
        *
        * @param route  ��������� �� ����� ���� ����� ����������
        *               ������������� ��������
        *               ����� ������ ����� ������ �� ������
        *               MAX_ROUTE_ID_TYPE_LENGTH+1
        */
        inline void getRouteId(char* route) const
        {
            getStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ������������� ��������
        *
        * return ������������� ��������
        */
        inline const char* getRouteId() const
        {
          return routeId;
        };

        /**
        * ����� ������������� ������������� sme-���������.
        *
        * @param id ������������� sme-���������
        */
        inline void setSourceSmeId(const char* id)
        {
            setStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ������������� sme-���������
        *
        * @param id     ��������� �� ����� ���� ����� ����������
        *               ������������� sme-���������
        *               ����� ������ ����� ������ �� ������
        *               MAX_SMESYSID_TYPE_LENGTH+1
        */
        inline void getSourceSmeId(char* id) const
        {
            getStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ������������� sme-���������
        *
        * return ������������� sme-���������
        */
        inline const char* getSourceSmeId() const
        {
          return srcSmeId;
        };

        /**
        * ����� ������������� ������������� sme-��������.
        *
        * @param id ������������� sme-��������
        */
        inline void setDestinationSmeId(const char* id)
        {
          setStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
        };

        /**
        * ����� ���������� ������������� sme-��������
        *
        * @param id     ��������� �� ����� ���� ����� ����������
        *               ������������� sme-��������
        *               ����� ������ ����� ������ �� ������
        *               MAX_SMESYSID_TYPE_LENGTH+1
        */
        inline void getDestinationSmeId(char* id) const
        {
            getStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
        };
        /**
        * ����� ���������� ������������� sme-��������
        *
        * return ������������� sme-��������
        */
        inline const char* getDestinationSmeId() const
        {
          return dstSmeId;
        };

#define ___SMS_DICT messageBody
        virtual void setStrProperty(const string& s,const string& value)
        {
          ___SMS_DICT.setStrProperty(s,value);
        }
        virtual void setIntProperty(const string& s,const uint32_t value)
        {
          ___SMS_DICT.setIntProperty(s,value);
        }
        virtual void setBinProperty(const string& s,const char* value,unsigned len)
        {
          ___SMS_DICT.setBinProperty(s,value,len);
        }
        virtual string getStrProperty(const string& s)
        {
          return ___SMS_DICT.getStrProperty(s);
        }
        virtual uint32_t getIntProperty(const string& s)
        {
          return ___SMS_DICT.getIntProperty(s);
        }
        virtual const char* getBinProperty(const string& s,unsigned* len)
        {
          return ___SMS_DICT.getBinProperty(s,len);
        }
        virtual bool hasIntProperty(const string& s)
        {
          return ___SMS_DICT.hasIntProperty(s);
        }
        virtual bool hasStrProperty(const string& s)
        {
          return ___SMS_DICT.hasStrProperty(s);
        }
        virtual bool hasBinProperty(const string& s)
        {
          return ___SMS_DICT.hasBinProperty(s);
        }
#undef ___SMS_DICT
        virtual ~SMS() {}
      };

  }}

#endif
