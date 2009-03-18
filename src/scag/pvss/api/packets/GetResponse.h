#ifndef _SCAG_PVSS_BASE_GETRESPONSE_H
#define _SCAG_PVSS_BASE_GETRESPONSE_H

#include "BatchResponseComponent.h"
#include "HavingProperty.h"

namespace scag2 {
namespace pvss {

class GetResponse : public BatchResponseComponent, public HavingProperty
{
public:
    GetResponse() : BatchResponseComponent(), property_(0) {}
    GetResponse( uint32_t seqNum ) : BatchResponseComponent(seqNum), property_(0) {}

    virtual ~GetResponse() {
        delete property_;
    }
    virtual const Property* getProperty() const { return property_; }
    virtual Property* getProperty() { return property_; }
    void setProperty( Property* prop ) {
        if ( property_ ) delete property_;
        property_ = prop;
    }

    virtual bool isValid() const {
        return (getStatus() != OK) || (property_ != 0 && property_->isValid());
    }
    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitGetResponse(*this);
    }

    virtual GetResponse* clone() const { return new GetResponse(*this); }

    void clear() {
        Response::clear();
        if ( property_ ) {
            delete property_;
            property_ = 0;
        }
    }

    /// disambiguation
    virtual bool isRequest() const { return Response::isRequest(); }
    virtual uint32_t getSeqNum() const { return Response::getSeqNum(); }

    // --- for serialization
    
    const char* getVarName() const {
        return property_->getName();
    }
    void setVarName( const std::string& varName ) {
        createProperty().setName( varName );
    }
    bool hasIntValue() const {
        return property_ != 0 && property_->getType() == INT;
    }
    bool hasStringValue() const {
        return property_ != 0 && property_->getType() == STRING;
    }
    bool hasDateValue() const {
        return property_ != 0 && property_->getType() == DATE;
    }
    bool hasBoolValue() const {
        return property_ != 0 && property_->getType() == BOOL;
    }
    int getIntValue() const {
        return property_->getIntValue();
    }
    const std::string& getStringValue() const {
        return property_->getStringValue();
    }
    bool getBoolValue() const {
        return property_->getBoolValue();
    }
    int getDateValue() const {
        return int(property_->getDateValue());
    }
    void setIntValue( int val ) {
        createProperty().setIntValue(val);
    }
    void setStringValue( const std::string& val ) {
        createProperty().setStringValue(val.c_str());
    }
    void setDateValue( int val ) {
        createProperty().setDateValue(time_t(val));
    }
    void setBoolValue( bool val ) {
        createProperty().setBoolValue(val);
    }

protected:
    virtual const char* typeToString() const { return "get_resp"; }

private:
    Property& createProperty() {
        if ( property_ == 0 ) property_ = new Property();
        return *property_;
    }

    GetResponse( const GetResponse& other ) : BatchResponseComponent(other), property_(0) {
        if ( other.property_ ) property_ = new Property(*other.property_);
    }
    GetResponse& operator = ( const GetResponse& other );

private:
    Property* property_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_GETRESPONSE_H */
