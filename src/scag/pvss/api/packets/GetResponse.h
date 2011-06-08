#ifndef _SCAG_PVSS_BASE_GETRESPONSE_H
#define _SCAG_PVSS_BASE_GETRESPONSE_H

#include "BatchResponseComponent.h"
#include "HavingProperty.h"

namespace scag2 {
namespace pvss {

class GetResponse : public BatchResponseComponent, public HavingProperty
{
public:
    GetResponse() : BatchResponseComponent() { initLog(); }
    GetResponse( uint8_t status ) : BatchResponseComponent(status) { initLog(); }
    virtual ~GetResponse() { logDtor(); }

    virtual const Property& getProperty() const { return property_; }
    virtual Property& getProperty() { return property_; }
    void setProperty( const Property& prop ) {
        CHECKMAGTC;
        property_ = prop;
    }

    virtual std::string toString() const {
        CHECKMAGTC;
        return BatchResponseComponent::toString() + " " + property_.toString();
    }


    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitGetResponse(*this);
    }

    virtual GetResponse* clone() const {
        CHECKMAGTC;
        return new GetResponse(*this); 
    }

    void clear() {
        CHECKMAGTC;
        BatchResponseComponent::clear();
        property_ = Property();
    }

    /// disambiguation
    virtual bool isRequest() const {
        CHECKMAGTC;
        return BatchResponseComponent::isRequest(); 
    }

    // --- for serialization
    
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

protected:
    virtual const char* typeToString() const { return "get_resp"; }

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

private:
    Property property_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETRESPONSE_H */
