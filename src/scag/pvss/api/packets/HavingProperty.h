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

    virtual const Property& getProperty() const = 0;
    virtual Property& getProperty() = 0;
    virtual void setProperty( const Property& ) = 0;

    virtual bool isRequest() const = 0;
    // virtual uint32_t getSeqNum() const = 0;

    // --- helpers for serialization
    const char* getVarName() const {
        return getProperty().getName();
    }
    void setVarName( const std::string& varName ) {
        getProperty().setName( varName );
    }
    bool hasIntValue() const {
        return getProperty().getType() == INT;
    }
    bool hasStringValue() const {
        return getProperty().getType() == STRING;
    }
    bool hasDateValue() const {
        return getProperty().getType() == DATE;
    }
    bool hasBoolValue() const {
        return getProperty().getType() == BOOL;
    }
    int getIntValue() const {
        return int(getProperty().getIntValue());
    }
    const std::string& getStringValue() const {
        return getProperty().getStringValue();
    }
    bool getBoolValue() const {
        return getProperty().getBoolValue();
    }
    int getDateValue() const {
        return int(getProperty().getDateValue());
    }
    void setIntValue( int val ) {
        getProperty().setIntValue(val);
    }
    void setStringValue( const std::string& val ) {
        getProperty().setStringValue(val.c_str());
    }
    void setDateValue( int val ) {
        getProperty().setDateValue(time_t(val));
    }
    void setBoolValue( bool val ) {
        getProperty().setBoolValue(val);
    }

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_HAVINGPROPERTY_H */
