#undef SMSDEFTAG
#if !defined SMS_IMPLEMENTATION
		#define SMSDEFTAG(tag_type,tag,x) static const char* x 
#else
	#if defined SMS_IMPLEMENTATION_STATIC
		#define SMSDEFTAG(tag_type,tag,x) const char* Tag::x = #x
	#else	
		#define SMSDEFTAG(tag_type,tag,x) \
			tagToKey[tag].type = tag_type;\
			tagToKey[tag].key = Tag::x;	\
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
#define SMS_BODY_UNKNOWN_TAG 2
#define ISMSDEFTAG(n,x) SMSDEFTAG(SMS_BODY_INT_TAG,n,x);
#define SSMSDEFTAG(n,x) SMSDEFTAG(SMS_BODY_STR_TAG,n,x);

#undef SMS_BODY_TAGS_SET
#define SMS_BODY_TAGS_SET \
	ISMSDEFTAG(0,SMPP_SCHEDULE_DELIVERY_TIME)\
	ISMSDEFTAG(1,SMPP_REPLACE_IF_PRESENT_FLAG)\
	ISMSDEFTAG(2,SMPP_ESM_CLASS)\
	ISMSDEFTAG(3,SMPP_DATA_CODING)\
	ISMSDEFTAG(4,SMPP_SM_LENGTH)\
	ISMSDEFTAG(5,SMPP_REGISTRED_DELIVERY)\
	ISMSDEFTAG(6,SMPP_PROTOCOL_ID)\
	SSMSDEFTAG(7,SMPP_SHORT_MESSAGE)\
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
	SSMSDEFTAG(20,SMPP_MESSAGE_PAYLOAD)

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
 
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <string>
#include <stdexcept>
#include <netinet/in.h>
#include <memory>
#include "core/buffers/Hash.hpp"

#include <util/debug.h>

namespace smsc { namespace sms
{
	using std::string;
	using smsc::core::buffers::Hash;
	using std::runtime_error;
	using std::auto_ptr;

		const int MAX_ESERVICE_TYPE_LENGTH = 5;
    const int MAX_ADDRESS_VALUE_LENGTH = 20;
    const int MAX_SHORT_MESSAGE_LENGTH = 200; //depricated
    const int MAX_BODY_LENGTH          = 2000; 
    // move it to SQL statements processing
    
    //const char* DEFAULT_ETSI_GSM_SEVICE_NAME = "GSM-SM";

    typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
    typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
    //typedef uint8_t     SMSData[MAX_SHORT_MESSAGE_LENGTH]; depricated
    typedef uint64_t    SMSId;

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
		virtual string getStrProperty(const string& s) = 0;
		virtual uint32_t getIntProperty(const string& s) = 0;
		virtual bool hasIntProperty(const string& s) = 0;
		virtual bool hasStrProperty(const string& s) = 0;
		virtual ~SMSDict(){}
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
			Hash<int> keyToTag;
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
      Hash<string> hash;
		public:
      int getRequiredBufferSize()
      {
        int size = 0;
				char* key;
        string* value;
        hash.First();
        while(hash.Next(key,value))
        {
          __require__(value!=0);
          size+=value->length()+4+1+2; 
        }
				__trace2__("TemporaryBodyStr size: %d",size);
				return size;
      }
      
      int hasValue(const string& key)
      {
        return hash.Exists(key.c_str());
      }
      
      string getValue(const string& key)
      {
        const string* value = hash.GetPtr(key.c_str());
        if ( !value ) return "";
        else return string(*value);
      }

      void setValue(const string& key,const string& value)
      {
        hash[key.c_str()] = value;
      }

      int encode(uint8_t* buffer,int offs,int length)
      {
        char* key;
        string* value;
        hash.First();
        while(hash.Next(key,value))
        {
          __require__(value!=0);
          __require__(offs+4<length);
          uint32_t len = (uint32_t)value->length()+1; 
          uint16_t tag = tag_hash.getTag(key);
          *(uint16_t*)(buffer+offs) = htons(tag);
          *(uint32_t*)(buffer+offs+2) = htonl(len);
          offs+=4+2;
					__trace2__("Senc: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key:"NULL",len,offs,length);
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
          uint16_t tag = ntohs(*(uint16_t*)(buffer+pos));
          uint32_t len = ntohl(*(uint32_t*)(buffer+pos+2));
          pos+=4+2;
          __require__(pos+len<=(unsigned)length);
					string* key = tag_hash.getStrKeyForString(tag);
					__trace2__("Sdec: tag=%hd key=%s len=%hd pos=%d length=%d",tag,key?key->c_str():"NULL",len,pos,length);
					if ( key )
					{
            __require__(*(buffer+pos+len-1) == 0);
            hash[key->c_str()] = string((char*)buffer+pos);
          }
					pos+=len;
        }
      }
    };

    class TemporaryBodyInt
    {
      Hash<int> hash;
		public:
      int getRequiredBufferSize()
      {
        int size = 0;
				char* key;
        int* value;
        hash.First();
        while(hash.Next(key,value))
        {
          //__reqruire__(value!=0);
          //size+=value->length(); 
					size+=4+4+2;
        }
				__trace2__("TemporaryBodyInt size: %d",size);
				return size;
      }
      
      bool hasValue(const string& key)
      {
        return hash.Exists(key.c_str())!=0;
      }
      
      int getValue(const string& key)
      {
        const int* value = hash.GetPtr(key.c_str());
        if ( !value ) return 0;
        else return *value;
      }

      void setValue(const string& key,int value)
      {
        hash[key.c_str()] = value;
      }

      int encode(uint8_t* buffer,int offs,int length)
      {
        char* key;
        int* value;
        hash.First();
        while(hash.Next(key,value))
        {
          __require__(value!=0);
          __require__(offs+4<length);
          uint32_t len = 4;
          uint16_t tag = tag_hash.getTag(key);
          *(uint16_t*)(buffer+offs) = htons(tag);
          *(uint32_t*)(buffer+offs+2) = htonl(4);
          offs+=4+2;
					__trace2__("Ienc: tag=%hd key=%s len=%hd pos=%d length=%d val=%d",tag,key?key:"NULL",len,offs,length,*value);
          __require__(offs+len<=(unsigned)length);
          //memcpy(buffer+pos,value->c_str(),len);
					*(uint32_t*)(buffer+offs) = htonl(*value); 
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
          uint16_t tag = ntohs(*(uint16_t*)(buffer+pos));
          uint32_t len = ntohl(*(uint32_t*)(buffer+pos+2));
          pos+=4+2;
          __require__(pos+len<=(unsigned)length);
          string* key = tag_hash.getStrKeyForInt(tag);
          if ( key )
          {
						__trace2__("Idec: tag=%hd key=%s len=%hd pos=%d length=%d val=%d",tag,key?key->c_str():"NULL",len,pos,length,ntohl(*(uint32_t*)(buffer+pos)));
            __require__(len == 4);
            hash[key->c_str()] = ntohl(*(uint32_t*)(buffer+pos));
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
          int len = body.getBufferLength();
					uint8_t* b = new uint8_t[len];
					memcpy(b,body.getBuffer(),len);
          setBuffer(b, len);
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
          int len = body.getBufferLength();
					uint8_t* b = new uint8_t[len];
					memcpy(b,body.getBuffer(),len);
          setBuffer(b, len);
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
        
        uint8_t* getBuffer()	const
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
					this->buff = buffer;
          //delete buffer;
        }

        void encode(uint8_t* buffer,int& length) const
        {
          __require__( buffer != 0 );  
          __require__( length >= 0 );
          __require__( length >= getRequiredBufferSize() );
          int offs;
          offs = temporaryBodyStr.encode(buffer,0,length);
          offs = temporaryBodyInt.encode(buffer,offs,length);
        };
        
        int getRequiredBufferSize() const
        {
          int size = temporaryBodyStr.getRequiredBufferSize()+
                 temporaryBodyInt.getRequiredBufferSize();
					__trace2__("RequiredBufferSize: %d",size);
					return size;
        };
        
        void decode(uint8_t* buffer,int length)
        {
          //__require__( buffer != 0 );  
          __require__( length >= 0 );
          temporaryBodyStr.decode(buffer,length);
          temporaryBodyInt.decode(buffer,length);
        };

        void setStrProperty(const string& s,const string& value) 
        {
          temporaryBodyStr.setValue(s,value);
        }
        
        void setIntProperty(const string& s,const uint32_t value) 
        {
          temporaryBodyInt.setValue(s,value);
        }
        
        string getStrProperty(const string& s)
        {
          return temporaryBodyStr.getValue(s);
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
        /*                      !!! Depricated !!!
        
        State       state;
        uint16_t    messageReference;
        
        Address     originatingAddress;
        Address     destinationAddress;
        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;
        
        time_t      waitTime;       // �����/���� � �������� �������� ���������
        time_t      validTime;      // �����/���� �� �������� ��������� �������
        time_t      submitTime;     // �����/���� ����������� �� SMSC
        time_t      lastTime;       // �����/���� ��������� ������� ��������
        time_t      nextTime;       // �����/���� ��������� ������� ��������
        
        uint8_t     priority;
        uint8_t     protocolIdentifier;
        
        uint8_t     deliveryReport;
        bool        needArchivate;
        
        uint8_t     failureCause;   // ������� ������� ��������� �������
        uint32_t    attempts;       // ���������� ���������� ������� ��������
        
        Body        messageBody;    // �������������� & ������ ���� ���������
        EService    eServiceType;   

        SMSId       receiptSmsId;   // id ��������� �� ������� ��� �����-������
        uint8_t     esmClass;       // ��� ���������: ����������, ����� ... 
        */

        State       state;
        time_t      submitTime;     // �����/���� ����������� �� SMSC
        time_t      validTime;      // �����/���� �� �������� ��������� �������
        
        uint32_t    attempts;       // ���������� ���������� ������� ��������
        uint8_t     lastResult;     // ��������� ��������� �������
        time_t      lastTime;       // �����/���� ��������� ������� ��������
        time_t      nextTime;       // �����/���� ��������� ������� ��������
        
        Address     originatingAddress;
        Address     destinationAddress;
				Address 		dealiasedDestinationAddress;

        uint16_t    messageReference;
        EService    eServiceType;   

        bool        needArchivate;
        uint8_t     deliveryReport;
        uint8_t     billingRecord;
        
        Descriptor  originatingDescriptor;
        Descriptor  destinationDescriptor;
        
        Body        messageBody;    // ���� ��������� + PDU ����.
        bool        attach;

        /**
         *                      !!! Depricated !!!                  
         *
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         *
        SMS() : state(ENROUTE), lastTime(0), nextTime(0),
                failureCause(0), attempts(0), receiptSmsId(0), esmClass(0)
        {
            eServiceType[0]='\0';
        };*/

        /**
         * Default �����������, ������ �������������� ���� state ��� ENROUTE
         * � ������ ���� ���������� ����������
         */
        SMS() : state(ENROUTE), submitTime(0), validTime(0),
                attempts(0), lastResult(0), lastTime(0), nextTime(0),
                messageReference(0), needArchivate(true),
                deliveryReport(0), billingRecord(0), attach(false)
        {
            eServiceType[0]='\0';
        }; 

        
        /**
         *                      !!! Depricated !!!                  
         *
         * ����������� �����������, ������������ ��� ��������
         * SMS �� �������
         * 
         * @param sms    ������� SMS
         *
        SMS(const SMS& sms) :
            state(sms.state), 
            messageReference(sms.messageReference),
            originatingAddress(sms.originatingAddress),
            destinationAddress(sms.destinationAddress), 
            originatingDescriptor(sms.originatingDescriptor),
            destinationDescriptor(sms.destinationDescriptor), 
            waitTime(sms.waitTime), validTime(sms.validTime), 
            submitTime(sms.submitTime), lastTime(sms.lastTime),
            nextTime(sms.nextTime), priority(sms.priority),
            protocolIdentifier(sms.protocolIdentifier),
            deliveryReport(sms.deliveryReport),
            needArchivate(sms.needArchivate),
            failureCause(sms.failureCause),
            attempts(sms.attempts),
            messageBody(sms.messageBody),
            receiptSmsId(sms.receiptSmsId),
            esmClass(sms.esmClass)
        {
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
        };*/
        
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
            attach(sms.attach)
        {
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
        };

        
        /**
         *                      !!! Depricated !!!                  
         *
         * ��������������� �������� '=',
         * ������������ ��� ����������� ���������
         * 
         * @param sms   ������ ����� ��������� '='
         * @return ������ �� ����
         *
        SMS& operator =(const SMS& sms) 
        {
            state = sms.state; 
            originatingAddress = sms.originatingAddress;
            destinationAddress = sms.destinationAddress;
            originatingDescriptor = sms.originatingDescriptor;
            destinationDescriptor = sms.destinationDescriptor;
            waitTime = sms.waitTime; validTime = sms.validTime; 
            submitTime = sms.submitTime; lastTime = sms.lastTime;
            nextTime = sms.nextTime; priority = sms.priority; 
            messageReference = sms.messageReference; 
            protocolIdentifier = sms.protocolIdentifier;
            deliveryReport = sms.deliveryReport;
            needArchivate = sms.needArchivate;
            failureCause = sms.failureCause;
            attempts = sms.attempts; 
            messageBody = sms.messageBody;
            receiptSmsId = sms.receiptSmsId;
            esmClass = sms.esmClass;
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
            return (*this);
        };*/

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
            
            strncpy(eServiceType, sms.eServiceType, sizeof(EService));
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
         *                      !!! Depricated !!!                  
         *
         * ������������� ����� ��������. 
         * 
         * @param time   ����, ����� ��������� ������ ���� ����������
         *               (�� �������� ��������).
         */
        /*inline void setWaitTime(time_t time) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //waitTime = time;
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ����� ��������.
         * 
         * @return ����, ����� ��������� ������ ���� ����������
         *         (�� �������� ��������).
         */
        /*inline time_t getWaitTime() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return waitTime;
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
         *                      !!! Depricated !!!                  
         *
         * ������������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pri    ��������� ���������
         */
        /*inline void setPriority(uint8_t pri) 
        {
            //__trace__("Method is depricated !!!");
            //__require__(false);
            //priority = pri;
					setIntProperty(SMPP_PRIORITY,(uint32_t)pri);
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ��������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ��������� ���������
         */
        /*inline uint8_t getPriority() const 
        {
            //__trace__("Method is depricated !!!");
            //__require__(false);
            //return priority;
					return (uint8_t)getIntProperty(SMPP_PRIORITY);
        };*/
       
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @param pid    ������ �������� ���������
         */
        /*inline void setProtocolIdentifier(uint8_t pid) 
        {
            //__trace__("Method is depricated !!!");
            //__require__(false);
            //protocolIdentifier = pid;
					setIntProperty(SMPP_PROTOCOL_ID,(uint32_t)pid);
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ������ �������� ���������.
         * � ������ ��������� SMS �� ������������, �� ����� ��� SMPP
         * 
         * @return ������ �������� ���������
         */
        /*inline uint8_t getProtocolIdentifier() const 
        {
            //__trace__("Method is depricated !!!");
            //__require__(false);
            //return protocolIdentifier;
					return (uint8_t)getIntProperty(SMPP_PROTOCOL_ID);
        };*/
       
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
         * ������������� ������� � ������ 
         * ������/��������������/���������� ���������
         * 
         * @param cause  ������� ������/��������������/���������� ���������
         *
        inline void setFailureCause(uint8_t cause) 
        {
            failureCause = cause;
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� ������� � ������
         * ������/��������������/���������� ���������
         * 
         * @return ������� ������/��������������/���������� ���������
         */
        /*inline uint8_t getFailureCause() const 
        {
            __trace__("Method is depricated !!!");
            //return failureCause;
            return getLastResult();
        };*/
        
        /**
         * ���������� ������� � ������
         * ������/��������������/���������� ��������� ������� �������� ���������
         * 
         * @return ������� ������/��������������/���������� ���������
         */
        inline uint8_t getLastResult() const 
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
         * ����������� ���������� ������ ��� �������� ��������� 
         * �� ��������� �������� (�� ��������� 1)
         * 
         * @param count  ���������� ������ ��� �������� ���������
         *
        inline void incrementAttemptsCount(uint32_t count=1) 
        {
            attempts += count;
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
         *                      !!! Depricated !!!                  
         *
         * ������������� ���� ���������
         * 
         * @param length ������ ���� ���������
         * @param scheme ����� ��������� ���� ���������
         * @param header �������, �������� �� ���� ���������
         * @param buff   �������������� ������ � ���� ��������� 
         * @see Body
         */
        //inline void setMessageBody(uint8_t length, uint8_t scheme, 
        //                           bool header, const uint8_t* buff) 
        //{ // Copies body data from 'buff' to static structure 
        //    /*messageBody.setCodingScheme(scheme);
        //    messageBody.setHeaderIndicator(header);
        //    messageBody.setData(length, buff);*/
        //    __trace__("Method is depricated !!!");
        //    setMessageBody((uint8_t *)buff, length); 
        //};
        
        /**
         * ������������� ���� ���������
         * 
         * @param buff   �������������� ������ � ���� ��������� 
         * @param length ������ ���� ���������
         *
         * @see Body
         */
        //inline void setMessageBody(uint8_t* buff, int length) 
        //{ // Copies body data from 'buff' to static structure 
        //    messageBody.setBuffer(buff, length);
        //};

        
        /**
         * ������������� ���� ���������
         * 
         * @param body ���� ���������
         * @see Body
         */
        //inline void setMessageBody(const Body& body) 
        //{ // Copies body data from 'body' to static structure 
        //    messageBody = body;     
        //};
        
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
        
        /**
         * ����� ������������� ���-��� ������� SME.
         * 
         * @param _len   ������ ������ �����-����
         * @param _name ���-��� SME
         */
        inline void setEServiceType(const char* _name) 
        {
            if (_name)
            {
                strncpy(eServiceType, _name, MAX_ESERVICE_TYPE_LENGTH);
                eServiceType[MAX_ESERVICE_TYPE_LENGTH]='\0';
            }
            else
            {
                eServiceType[0]='\0';
            }
        };
        
        /**
         * ����� �������� ���-��� ������� SME � ���������� ��� ������
         * 
         * @param _name ��������� �� ����� ���� ����� ������������ ���
         *              ����� ������ ����� ������ �� ������
         *              MAX_ESERVICE_TYPE_LENGTH+1, ����� ������� 
         *              ����� ��������
         */
        inline void getEServiceType(char* _name) const 
        {
            __require__(_name);
            
            if (eServiceType)
            {
                strncpy(_name, eServiceType, MAX_ESERVICE_TYPE_LENGTH);
                _name[MAX_ESERVICE_TYPE_LENGTH]='\0';
            }
            else
            {
                _name[0]='\0';
            }
        };
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ������������� id ���������, �� ������� ��� �����-������.
         * ������������ � ������ ���� esmClass ��� �����-������.
         * 
         * @param id     ����������������� ����� ���������
         */
        /*inline void setReceiptSmsId(SMSId id) 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //receiptSmsId = id;
        };*/
        
        /**
         *                      !!! Depricated !!!                  
         *
         * ���������� id ���������, �� ������� ��� �����-������.
         * ������������ � ������ ���� esmClass ��� �����-������.
         * 
         * @return ����������������� ����� ��������� (MR)
         */
        /*inline SMSId getReceiptSmsId() const 
        {
            __trace__("Method is depricated !!!");
            __require__(false);
            //return receiptSmsId;
        };*/
        
        /**
         * ������������� ���-���������.
         * 
         * @param type      ���-���������    
         */
        /*inline void setEsmClass(uint8_t type) 
        {
        };*/
        
        /**
         * ���������� ���-���������.
         * 
         * @return ���-���������
         */
        /*inline uint8_t getEsmClass() const 
        {
        };*/
    
				#define ___SMS_DICT messageBody
				virtual void setStrProperty(const string& s,const string& value)
				{
					___SMS_DICT.setStrProperty(s,value);
				}
				virtual void setIntProperty(const string& s,const uint32_t value)
				{
					___SMS_DICT.setIntProperty(s,value);
				}
				virtual string getStrProperty(const string& s)
				{
					return ___SMS_DICT.getStrProperty(s);
				}
				virtual uint32_t getIntProperty(const string& s)
				{
					return ___SMS_DICT.getIntProperty(s);
				}
				virtual bool hasIntProperty(const string& s)
				{
					return ___SMS_DICT.hasIntProperty(s);
				}
				virtual bool hasStrProperty(const string& s)
				{
					return ___SMS_DICT.hasStrProperty(s);
				}
				#undef ___SMS_DICT
				virtual ~SMS() {}
    };

}}

#endif


