/* ************************************************************************* *
 * Base classes that implements generic ASN.1 types and ABSTRACT-SYNTAX class
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TYPE_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TYPE_DEFS__

#include <stdarg.h>
#include "eyeline/asn1/EncodedOID.hpp"

namespace eyelinecom {
namespace asn1 {

struct OCTBuffer {
    uint8_t *       ptr;
    uint32_t        size;

    OCTBuffer(uint8_t * use_ptr = 0, uint32_t use_sz = 0)
        : ptr(use_ptr), size(use_sz)
    { }
};

//unaligned BIT buffer
struct BITBuffer : public OCTBuffer {
    uint8_t         bitsGap;    //unused bits in first byte of encoding
    uint8_t         bitsUnused; //unused bits in last byte of encoding

    BITBuffer(uint8_t * use_ptr = 0, uint32_t use_sz = 0)
        : OCTBuffer(use_ptr, use_sz)
        , bitsUnused(0), bitsGap(0)
    { }
};

struct ASTag {
    enum TagClass {
        tagUniversal = 0x00,
        tagApplication = 0x40,
        tagContextSpecific = 0x80,
        tagPrivate = 0xB0
    };

    TagClass    tagClass;
    uint16_t    tagValue;

    ASTag(TagClass tag_class = tagUniversal, uint16_t tag_val = 0)
        : tagClass(tag_class), tagValue(tag_val)
    { }
};

//basic abstract type tagging
class ASTypeTags : std::vector<ASTag> {
public:
    ASTypeTags()
    { }
    ASTypeTags(const ASTypeTags & use_tags)
        : std::vector<ASTag>(use_tags)
    { }
    ASTypeTags(const ASTag & use_tag) //just one tag
        : std::vector<ASTag>(1)
    {
        at(0) = use_tag;
    }
    ASTypeTags(uint16_t num_tags, ASTag use_tag1, ...
                                /* , const ASTag use_tagN*/)
        : std::vector<ASTag>(num_tags)
    {
        at(0) = use_tag1;
        va_list  useTags;
        va_start(useTags, use_tag1);
        for (uint16_t i = 1; i < num_tags; ++i)
            at(i) = va_arg(useTags, ASTag);
        va_end(useTags);
    }
    ~ASTypeTags()
    { }


    inline uint16_t numTags(void) const
    {
        return (uint16_t)(size());
    }
     //tag_idx = 0 - outermost tag, in most cases this is just a type tag
    inline const ASTag & Tag(uint16_t tag_idx = 0) const
    {
        return tag_idx < numTags() ? at(tag_idx) : 0;
    }

};

//complex abstract type (CHOICE) tagging
class ASTypeTagging : std::vector<ASTypeTags> {
public:
    ASTypeTagging()
    { }
    ASTypeTagging(const ASTypeTagging & use_tagging)
        : std::vector<ASTypeTags>(use_tagging)
    { }
    ASTypeTagging(const ASTag & use_tag) //type is tagged by single tag
        : std::vector<ASTypeTags>(1)
    {
        at(0) = ASTypeTags(use_tag);
    }
    ASTypeTagging(const ASTypeTags & use_tags)
        : std::vector<ASTypeTags>(1)
    {
        at(0) = use_tags;
    }
    ASTypeTagging(uint16_t num_tags, ASTypeTags use_tags1, ...
                                    /* , ASTypeTags use_tagsN*/)
        : std::vector<ASTypeTags>(num_tags)
    {
        at(0) = use_tags1;
        va_list  useTags;
        va_start(useTags, use_tags1);
        for (uint16_t i = 1; i < num_tags; ++i) {
            at(i) = va_arg(useTags, ASTypeTags);
        }
        va_end(useTags);
    }
    ~ASTypeTagging()
    { }

    inline uint16_t numOptions(void) const { return (uint16_t)size(); }
    inline uint16_t numTags(uint16_t option_idx = 0) const
    {
        return option_idx < numOptions() ? at(option_idx).numTags(option_idx) : 0;
    }
    //0:0 - outermost tag of first tagging option, in most cases this is just a type tag
    inline const ASTag & Tag(uint16_t tag_idx = 0, uint16_t option_idx = 0) const
    {
        return option_idx < numOptions() ? at(option_idx).Tag(tag_idx) : ASTag();
    }
};

//Generic abstract type, implements ABSTRACT-SYNTAX.&Type
class ASTypeAC {
public:
    enum Presentation {
        valNone = 0, valEncoded, valMixed, valDecoded
    };
    enum EncodingRule {
        undefinedER = 0
        //octet aligned encodings:
        , ruleBER   //ruleBasic
        , ruleDER   //ruleDistinguished
        , ruleCER   //ruleCanonical
        , rulePER   //rulePacked_aligned
        , ruleCPER  //rulePacked_aligned_canonical
        , ruleXER   //ruleXml
        , ruleCXER  //ruleXml_canonical
        , ruleEXER  //ruleXml_extended
        //bit aligned encodings:
        , ruleUPER  //rulePacked_unaligned
        , ruleCUPER //rulePacked_unaligned_canonical
    };

    enum ENCStatus {
        encBadVal = -2, encError = -1, encOk = 0
    };
    enum DECStatus {
        decBadVal = -2, decError = -1, decOk = 0, decMoreInput = 1
    };

    struct ENCResult {
        ENCStatus   rval;   //encoding status
        uint16_t    nbytes; //number of bytes encoded

        EnCResult() : rval(encOk), nbytes(0)
    };
    struct DECResult {
        DECStatus   rval;   //decoding status
        uint16_t    nbytes; //number of bytes succsefully decoded

        DECResult() : rval(decOk), nbytes(0)
    };

protected:
    Presentation        valType;
    const BITBuffer *   valEnc;
    EncodingRule        valRule;
    ASTypeTagging       tags;

public:
    ASTypeAC()
        : valType(valNone), valEnc(0), valRule(undefinedER)
    { }
    ASTypeAC(const ASTypeTagging & use_tags)
        : valType(valNone), valEnc(0), valRule(undefinedER)
        , tags(use_tags)
    { }
    ASTypeAC(ASTag::TagClass tag_class,  uint16_t tag_val)
        : valType(valNone), valEnc(0), valRule(undefinedER)
        , tags(ASTypeTagging(ASTag(tag_class, tag_val)))
    { }
    virtual ~ASTypeAC()
    { }

    //type is tagged by single tag
    inline void setTagging(ASTag::TagClass tag_class,  uint16_t tag_val)
    {
        tags = ASTypeTagging(ASTag(tag_class, tag_val));
    }
    inline void setTagging(const ASTypeTagging & use_tags)
    {
        tags = use_tags;
    }
    inline void setEncoding(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    {
        valType = valEncoded; valBuf = &use_buf; valRule = use_rule;
    }

    //0:0 - outermost tag of first tagging option, in most cases this is just a type tag
    inline const ASTag & Tag(uint16_t tag_idx = 0, uint16_t option_idx = 0) const
    {
        return tags.Tag(tag_idx, option_idx);
    }
    inline const ASTypeTagging & Tagging(void) const { return tags; }

    inline Presentation getPresentation(void) const { return valType; }

    inline const BITBuffer * getEncoding(void) const { return valBuf; }

    inline EncodingRule getRule(void) const { return valRule; }

    // ---------------------------------
    // ASTypeAC interface methods
    // ---------------------------------

    //REQ: presentation > valNone, if use_rule == valRule, otherwise presentation == valDecoded
    virtual ENCResult Encode(BITBuffer & buf, EncodingRule use_rule = ruleDER) /*throw ASN1CodecError*/ = 0;
    //REQ: presentation == valEncoded | valMixed (setEncoding was called)
    //OUT: type presentation = valDecoded, components (if exist) presentation = valDecoded,
    //in case of decMoreInput, stores decoding context
    virtual DECResult Decode(void) /*throw ASN1CodecError*/ = 0;
    //REQ: presentation == valEncoded  (setEncoding was called)
    //OUT: type presentation = valMixed | valDecoded, 
    //     deferred components presentation = valEncoded
    //NOTE: if num_tags == 0, all components decoding is deferred 
    //in case of decMoreInput, stores decoding context 
    virtual DECResult Demux(uint16_t num_tags = 0,
                            const ASTypeTagging (* defer_tag)[] = NULL) /*throw ASN1CodecError*/ = 0;


    // ---------------------------------
    // ASTypeAC auxiliary methods
    // ---------------------------------
    //REQ: presentation == valNone, if decMoreInput stores decoding context,
    //type presentation = valDecoded, components (if exist) presentation = valDecoded
    inline DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
        /*throw ASN1CodecError*/
    {
        setEncoding(use_buf, use_rule);
        return Decode(use_flags);
    }
    //REQ: presentation == valNone, if decMoreInput stores decoding context
    //type presentation = valDecoded, deferred components presentation = valEncoded
    //NOTE: if num_tags == 0 all components are deferred 
    inline DECResult Demux(const BITBuffer & buf, EncodingRule use_rule = ruleDER,
                    uint16_t num_tags = 0, const ASTypeTagging (* defer_tag)[] = NULL)
        /*throw ASN1CodecError*/
    {
        setEncoding(use_buf, use_rule);
        return Decode(use_flags, num_tags, defer_tag);
    }
};


class AbstractSyntax : public ASTypeAC {
public
    const EncodedOID &  _asId; //associated ABSTRACT-SYNTAX.&id

    AbstractSyntax(const EncodedOID & use_Id)
        : _asId(use_asId), ASTypeAC()
    { }
    AbstractSyntax(const EncodedOID & use_Id, ASTag::TagClass tag_class,  uint16_t tag_val)
        : _asId(use_asId), ASTypeAC(tag_class, tag_val)
    { }
    AbstractSyntax(const EncodedOID & use_Id, const ASTypeTagging & use_tags)
        : _asId(use_asId), ASTypeAC(use_tags)
    { }

};

}; //asn1
}; //eyelinecom

#endif /* __ABSTRACT_SYNTAX_TYPE_DEFS__ */

