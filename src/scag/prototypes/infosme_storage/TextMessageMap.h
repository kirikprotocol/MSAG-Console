#ifndef _SCAG_PROTOTYPES_INFOSME_TEXTMESSAGEMAP_H
#define _SCAG_PROTOTYPES_INFOSME_TEXTMESSAGEMAP_H

namespace scag2 {
namespace prototypes {
namespace infosme {

/// external locking is required.
class TextMessageMap
{
public:
    /// get the real textId corresponding to the input textId or 0.
    uint32_t getRealTextId( uint32_t inputTextId ) const;

    /// get the text corresponding to the textId or 0.
    const char* getText( uint32_t textId ) const;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_TEXTMESSAGEMAP_H */
