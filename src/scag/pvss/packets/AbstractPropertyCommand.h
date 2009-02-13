#ifndef _SCAG_PVSS_BASE_ABSTRACTPROPERTYCOMMAND_H
#define _SCAG_PVSS_BASE_ABSTRACTPROPERTYCOMMAND_H

#include "HavingProperty.h"
#include "BatchRequestComponent.h"
#include "ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class AbstractPropertyCommand : public BatchRequestComponent, public HavingProperty
{
protected:
    AbstractPropertyCommand() : BatchRequestComponent(), property_(0) {}
    AbstractPropertyCommand( uint32_t seqNum ) : BatchRequestComponent(seqNum), property_(0) {}

public:
    virtual ~AbstractPropertyCommand() {
        delete property_;
    }

    const Property* getProperty() const {
        return property_;
    }
    Property* getProperty() {
        return property_;
    }
    void setProperty( Property* prop ) {
        if ( property_ ) delete property_;
        property_ = prop;
    }
    bool isValid() const {
        return property_ != 0 && property_->isValid();
    }
    void clear() {
        if ( property_ ) {
            delete property_;
            property_ = 0;
        }
    }

    std::string toString() const {
        return BatchRequestComponent::toString() + " " + ( property_ ? property_->toString() : "NULL" );
    }

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

    /// disambiguate inherited methods isRequest() and getSeqNum().
    virtual bool isRequest() const {
        return BatchRequestComponent::isRequest();
    }
    virtual uint32_t getSeqNum() const {
        return BatchRequestComponent::getSeqNum();
    }

private:
    Property& createProperty() {
        if ( property_ == 0 ) property_ = new Property();
        return *property_;
    }


protected:
    Property*  property_;  // owned
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTCOMMAND_H */
