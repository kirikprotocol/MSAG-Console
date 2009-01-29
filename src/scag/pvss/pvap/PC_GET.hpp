// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_GET_HPP__
#define __SCAG_PVSS_PVAP_PC_GET_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"
#include "TypeId.h"


namespace scag{
namespace pvss{
namespace pvap{

// class PVAP;

class PC_GET  
{
public:
    PC_GET()
    {
        clear();
    }
    void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        varNameFlag=false;
    }

    std::string toString() const
    {
        std::string rv("PC_GET:");
        char buf[32];
        sprintf(buf,"seqNum=%d",seqNum);
        rv+=buf;
        if(profileTypeFlag) {
            rv+=";profileType=";
            sprintf(buf,"%u",(unsigned int)profileType);
            rv+=buf;
        }
        if(abonentKeyFlag) {
            rv+=";abonentKey=";
            rv+=abonentKey;
        }
        if(profileKeyFlag) {
            rv+=";profileKey=";
            sprintf(buf,"%u",(unsigned int)profileKey);
            rv+=buf;
        }
        if(varNameFlag) {
            rv+=";varName=";
            rv+=varName;
        }
        return rv;
    }

    /*
    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        if (profileTypeFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(profileType);
        }
        if (abonentKeyFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(abonentKey);
        }
        if (profileKeyFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(profileKey);
        }
        if (varNameFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(varName);
        }
        return rv;
    }
     */

    uint8_t getProfileType() const
        throw (FieldIsNullException)
    {
        if (!profileTypeFlag) {
            throw FieldIsNullException("profileType");
        }
        return profileType;
    }
    void setProfileType(uint8_t value)
    {
        profileType=value;
        profileTypeFlag=true;
    }
    bool hasProfileType()const
    {
        return profileTypeFlag;
    }
    const std::string& getAbonentKey() const
        throw (FieldIsNullException)
    {
        if (!abonentKeyFlag) {
            throw FieldIsNullException("abonentKey");
        }
        return abonentKey;
    }
    void setAbonentKey(const std::string& value)
    {
        abonentKey=value;
        abonentKeyFlag=true;
    }
    bool hasAbonentKey()const
    {
        return abonentKeyFlag;
    }
    uint32_t getProfileKey() const
        throw (FieldIsNullException)
    {
        if (!profileKeyFlag) {
            throw FieldIsNullException("profileKey");
        }
        return profileKey;
    }
    void setProfileKey(uint32_t value)
    {
        profileKey=value;
        profileKeyFlag=true;
    }
    bool hasProfileKey()const
    {
        return profileKeyFlag;
    }
    const std::string& getVarName() const
        throw (FieldIsNullException)
    {
        if (!varNameFlag) {
            throw FieldIsNullException("varName");
        }
        return varName;
    }
    void setVarName(const std::string& value)
    {
        varName=value;
        varNameFlag=true;
    }
    bool hasVarName()const
    {
        return varNameFlag;
    }

    template <class Proto, class DataStream>
        void serialize( const Proto& proto, DataStream& ds ) const throw (PvapException)
    {
        checkFields();
        // mandatory fields
        printf( "write pos=%d field=%d\n", ds.getPos(), profileTypeTag );
        ds.writeTag(profileTypeTag);
        ds.writeByteLV(profileType);
        printf( "write pos=%d field=%d\n", ds.getPos(), varNameTag );
        ds.writeTag(varNameTag);
        ds.writeByteStringLV(varName);
        // optional fields
        if (abonentKeyFlag) {
            printf( "write pos=%d field=%d\n", ds.getPos(), abonentKeyTag );
            ds.writeTag(abonentKeyTag);
            ds.writeByteStringLV(abonentKey);
        }
        if (profileKeyFlag) {
            printf( "write pos=%d field=%d\n", ds.getPos(), profileKeyTag );
            ds.writeTag(profileKeyTag);
            ds.writeInt32LV(profileKey);
        }
    }

    template <class Proto, class DataStream> void deserialize(const Proto& proto, DataStream& ds)
        throw (PvapException)
    {
        clear();
        while (true) {
            int pos = int(ds.getPos());
            int tag = ds.readTag();
            printf( "read pos=%d field=%d\n", pos, tag );
            if ( tag == -1 ) break;
            switch(tag) {
            case profileTypeTag: {
                if (profileTypeFlag) {
                    throw DuplicateFieldException("profileType");
                }
                profileType=ds.readByteLV();
                profileTypeFlag=true;
                break;
            }
            case abonentKeyTag: {
                if (abonentKeyFlag) {
                    throw DuplicateFieldException("abonentKey");
                }
                abonentKey=ds.readByteStringLV();
                abonentKeyFlag=true;
                break;
            }
            case profileKeyTag: {
                if (profileKeyFlag) {
                    throw DuplicateFieldException("profileKey");
                }
                profileKey=ds.readInt32LV();
                profileKeyFlag=true;
                break;
            }
            case varNameTag: {
                if (varNameFlag) {
                    throw DuplicateFieldException("varName");
                }
                varName=ds.readByteStringLV();
                varNameFlag=true;
                break;
            }
            default:
                throw NotImplementedException("reaction of reading unknown");
            }
        }
        checkFields();
    }

    uint32_t getSeqNum() const
    {
        return seqNum;
    }
 
    void setSeqNum(uint32_t value)
    {
        seqNum=value;
    }

protected:
    void checkFields() const throw (MandatoryFieldMissingException)
    {
        // checking mandatory fields
        if (!profileTypeFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "profileType", "PC_GET");
            throw MandatoryFieldMissingException(buf);
        }
        if (!varNameFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "varName", "PC_GET");
            throw MandatoryFieldMissingException(buf);
        }
        // checking optional fields
        if (!abonentKeyFlag
            && (profileType==1)
            ) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "abonentKey", "PC_GET");
            throw MandatoryFieldMissingException(buf);
        }
        if (!profileKeyFlag
            && (profileType>1)
            && (profileType<5)
            ) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "profileKey", "PC_GET");
            throw MandatoryFieldMissingException(buf);
        }
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    static const int profileTypeTag=2;
    static const int abonentKeyTag=3;
    static const int profileKeyTag=4;
    static const int varNameTag=5;

    uint32_t seqNum;

    uint8_t profileType;
    std::string abonentKey;
    uint32_t profileKey;
    std::string varName;

    bool profileTypeFlag;
    bool abonentKeyFlag;
    bool profileKeyFlag;
    bool varNameFlag;
};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
