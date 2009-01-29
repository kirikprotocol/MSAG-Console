// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_BATCH_HPP__
#define __SCAG_PVSS_PVAP_PC_BATCH_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"
#include "TypeId.h"
#include "BatchCmdArray.h"


namespace scag{
namespace pvss{
namespace pvap{

// class PVAP;

class PC_BATCH  
{
public:
    PC_BATCH()
    {
        clear();
    }
    void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        batchModeFlag=false;
        batchContentFlag=false;
    }

    std::string toString() const
    {
        std::string rv("PC_BATCH:");
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
        if(batchModeFlag) {
            rv+=";batchMode=";
            rv+=batchMode?"true":"false";
        }
        if(batchContentFlag) {
            rv+=";batchContent=";
            rv+=batchContent.toString();
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
        if (batchModeFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(batchMode);
        }
        if (batchContentFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(batchContent);
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
    bool getBatchMode() const
        throw (FieldIsNullException)
    {
        if (!batchModeFlag) {
            throw FieldIsNullException("batchMode");
        }
        return batchMode;
    }
    void setBatchMode(bool value)
    {
        batchMode=value;
        batchModeFlag=true;
    }
    bool hasBatchMode()const
    {
        return batchModeFlag;
    }
    const BatchCmdArray& getBatchContent() const
        throw (FieldIsNullException)
    {
        if (!batchContentFlag) {
            throw FieldIsNullException("batchContent");
        }
        return batchContent;
    }
    void setBatchContent(const BatchCmdArray& value)
    {
        batchContent=value;
        batchContentFlag=true;
    }
    bool hasBatchContent()const
    {
        return batchContentFlag;
    }

    template <class Proto, class DataStream>
        void serialize( const Proto& proto, DataStream& ds ) const throw (PvapException)
    {
        checkFields();
        // mandatory fields
        printf( "write pos=%d field=%d\n", ds.getPos(), profileTypeTag );
        ds.writeTag(profileTypeTag);
        ds.writeByteLV(profileType);
        printf( "write pos=%d field=%d\n", ds.getPos(), batchModeTag );
        ds.writeTag(batchModeTag);
        ds.writeBoolLV(batchMode);
        printf( "write pos=%d field=%d\n", ds.getPos(), batchContentTag );
        ds.writeTag(batchContentTag);
        batchContent.serialize(proto,ds);
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
            case batchModeTag: {
                if (batchModeFlag) {
                    throw DuplicateFieldException("batchMode");
                }
                batchMode=ds.readBoolLV();
                batchModeFlag=true;
                break;
            }
            case batchContentTag: {
                if (batchContentFlag) {
                    throw DuplicateFieldException("batchContent");
                }
                batchContent.deserialize(proto,ds);
                batchContentFlag=true;
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
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "profileType", "PC_BATCH");
            throw MandatoryFieldMissingException(buf);
        }
        if (!batchModeFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "batchMode", "PC_BATCH");
            throw MandatoryFieldMissingException(buf);
        }
        if (!batchContentFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "batchContent", "PC_BATCH");
            throw MandatoryFieldMissingException(buf);
        }
        // checking optional fields
        if (!abonentKeyFlag
            && (profileType==1)
            ) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "abonentKey", "PC_BATCH");
            throw MandatoryFieldMissingException(buf);
        }
        if (!profileKeyFlag
            && (profileType>1)
            && (profileType<5)
            ) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "profileKey", "PC_BATCH");
            throw MandatoryFieldMissingException(buf);
        }
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    static const int profileTypeTag=2;
    static const int abonentKeyTag=3;
    static const int profileKeyTag=4;
    static const int batchModeTag=21;
    static const int batchContentTag=23;

    uint32_t seqNum;

    uint8_t profileType;
    std::string abonentKey;
    uint32_t profileKey;
    bool batchMode;
    BatchCmdArray batchContent;

    bool profileTypeFlag;
    bool abonentKeyFlag;
    bool profileKeyFlag;
    bool batchModeFlag;
    bool batchContentFlag;
};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
