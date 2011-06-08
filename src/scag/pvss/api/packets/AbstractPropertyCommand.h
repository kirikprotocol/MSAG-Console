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
    AbstractPropertyCommand() : BatchRequestComponent() {}
    // AbstractPropertyCommand( uint32_t seqNum ) : BatchRequestComponent(seqNum) {}
    AbstractPropertyCommand( const AbstractPropertyCommand& cmd ) :
    BatchRequestComponent(cmd), HavingProperty(cmd), property_(cmd.property_) {
        // if ( cmd.property_ ) property_ = new Property(*cmd.property_);
    }

public:
    virtual ~AbstractPropertyCommand() {
        // delete property_;
    }

    const Property& getProperty() const {
        CHECKMAGTC;
        return property_;
    }
    Property& getProperty() {
        CHECKMAGTC;
        return property_;
    }
    void setProperty( const Property& prop ) {
        CHECKMAGTC;
        // if ( property_ ) delete property_;
        property_ = prop;
    }
    bool isValid( PvssException* exc = 0 ) const {
        CHECKMAGTC;
        if ( ! property_.isValid() ) {
            if ( exc ) *exc = PvssException(PvssException::BAD_REQUEST,
                                            "property %s is invalid", property_.toString().c_str() );
            return false;
        }
        return true;
    }
    void clear() {
        CHECKMAGTC;
        property_ = Property();
        /*
        if ( property_ ) {
            delete property_;
            property_ = 0;
        }
         */
    }

    std::string toString() const {
        CHECKMAGTC;
        return BatchRequestComponent::toString() + " " + property_.toString();
    }

    // --- for serialization
    
    const char* getVarName() const {
        return property_.getName();
    }
    void setVarName( const std::string& varName ) {
        createProperty().setName( varName );
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
    // virtual uint32_t getSeqNum() const {
    // return BatchRequestComponent::getSeqNum();
    // }

private:
    Property& createProperty() {
        CHECKMAGTC;
        // if ( property_ == 0 ) property_ = new Property();
        return property_;
    }

protected:
    Property  property_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTCOMMAND_H */
