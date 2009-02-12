#ifndef _SCAG_PVSS_BASE_HAVINGPROPERTY_H
#define _SCAG_PVSS_BASE_HAVINGPROPERTY_H

#include "util/int.h"
#include "scag/pvss/base/Property.h"

namespace scag2 {
namespace pvss {

/// Interface HavingProperty
class HavingProperty
{
public:
    virtual ~HavingProperty() {}

    virtual const Property* getProperty() const = 0;
    virtual Property* getProperty() = 0;
    virtual void setProperty( Property* ) = 0;

    virtual bool isRequest() const = 0;
    virtual uint32_t getSeqNum() const = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_HAVINGPROPERTY_H */
