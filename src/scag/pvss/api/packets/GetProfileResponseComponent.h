#ifndef _SCAG_PVSS_BASE_GETPROFILERESPONSECOMPONENT_H
#define _SCAG_PVSS_BASE_GETPROFILERESPONSECOMPONENT_H

#include "HavingProperty.h"

namespace scag2 {
namespace pvss {

class GetProfileResponseComponent
{
private:
    // static smsc::logger::Logger* log_;

public:
    GetProfileResponseComponent() {}
    GetProfileResponseComponent( const char* p ) : varname_(p) {}
    virtual ~GetProfileResponseComponent() {}

    // virtual const Property& getProperty() const { return property_; }
    // virtual Property& getProperty() { return property_; }
    // void setProperty( const Property& prop ) {
    // property_ = prop;
    // }

    std::string toString() const {
        return varname_;
    }

    /// disambiguation
    bool isRequest() const { return false; }

    GetProfileResponseComponent* clone() const {
        return new GetProfileResponseComponent(*this);
    }

    // --- for serialization
    
    const std::string& getVarName() const { return varname_; }
    void setVarName( const std::string& s ) { varname_ = s; }

    /*
    const char* getVarName() const {
        return property_.getName();
    }
    void setVarName( const std::string& varName ) {
        property_.setName( varName );
    }
    bool hasIntValue() const {
        return property_.getType() == INT;
    }
    bool hasStringValue() const {
        return property_.getType() == STRING;
    }
    bool hasDateValue() const {
        return property_.getType() == DATE;
    }
    bool hasBoolValue() const {
        return property_.getType() == BOOL;
    }
    int getIntValue() const {
        return int(property_.getIntValue());
    }
    const std::string& getStringValue() const {
        return property_.getStringValue();
    }
    bool getBoolValue() const {
        return property_.getBoolValue();
    }
    int getDateValue() const {
        return int(property_.getDateValue());
    }
    void setIntValue( int val ) {
        property_.setIntValue(val);
    }
    void setStringValue( const std::string& val ) {
        property_.setStringValue(val.c_str());
    }
    void setDateValue( int val ) {
        property_.setDateValue(time_t(val));
    }
    void setBoolValue( bool val ) {
        property_.setBoolValue(val);
    }
     */

    void clear() { varname_.clear(); }
    bool isValid() const { return !varname_.empty(); }

protected:
    // virtual const char* typeToString() const { return "get_resp"; }

private:
    /*
    Property& createProperty() {
        if ( property_ == 0 ) property_ = new Property();
        return *property_;
    }
     */

    // default is ok
    // GetResponse( const GetResponse& other ) : BatchResponseComponent(other), property_(other.property_) {
    // }
    // GetResponse& operator = ( const GetResponse& other );

    GetProfileResponseComponent( const GetProfileResponseComponent& o ) :
    varname_(o.varname_) {
    }

    GetProfileResponseComponent& operator = ( const GetProfileResponseComponent& );

    // void initLog();
    // void logDtor();

private:
    std::string varname_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETRESPONSE_H */
