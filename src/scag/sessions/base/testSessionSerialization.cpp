#include "logger/Logger.h"
#include "Session2.h"
#include "scag/util/io/Serializer.h"
#include "scag/util/io/HexDump.h"
#include "ExternalBillingTransaction.h"

using scag2::sessions::Session;
using scag2::sessions::SessionPropertyScope;
using scag2::sessions::SessionKey;
using scag2::sessions::ExternalTransaction;
using scag2::sessions::ExternalBillingTransaction;
using namespace scag2::util;
using namespace scag2::util::io;
using namespace scag2::transport;

class DummyScagCommand : public SCAGCommand
{
public:
    DummyScagCommand( TransportType tt,
                      int servId,
                      int cmdId ) :
    SCAGCommand(),
    transType_(tt), servId_(servId), cmdId_(cmdId), serial_(makeSerial()),
    opId_(invalidOpId()) {}

    virtual TransportType getType() const { return transType_; }

    virtual int getServiceId() const { return servId_; }
    virtual void setServiceId(int serviceId) { servId_ = serviceId; }

    // initially set to invalidOpId()
    virtual opid_type getOperationId() const { return opId_; }
    virtual void setOperationId( opid_type op) { opId_ = op; }
    /*
    static inline opid_type invalidOpId() {
        return 0;
    }
     */

    virtual uint8_t getCommandId() const { return cmdId_; }
      
    virtual uint32_t getSerial() const { return serial_; }

    /*
    virtual SessionPtr getSession() = 0;
    virtual void setSession(const SessionPtr&) = 0;
    virtual bool hasSession() = 0;
     */

    virtual Session* getSession() { return session_; }

    /// print-out the command (for debugging)
    virtual void print( Print& p ) const {
        p.print("cmd #%u, id=%u srv=%u op=%u",serial_,cmdId_,servId_,opId_);
    }

protected:
    /// session is set from session store when the command is locking the session.
    virtual void setSession( Session* as ) { session_ = as; }

private:
    TransportType transType_;
    int       servId_;
    int       cmdId_;
    uint32_t  serial_;
    opid_type opId_;
    Session*  session_;
};


int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger* logg = smsc::logger::Logger::getInstance("main");
    
    std::auto_ptr<Session> s;

    const ExternalBillingTransaction::billid_type billId = 78654;
    const SessionKey sk(std::string(".1.1.79137654079"));

    // ---
    {
        smsc_log_debug(logg,"making a session");
        s.reset( new Session(sk) );

        // adding transaction
        char buf[50];
        sprintf(buf,"msag.bill.%u",unsigned(billId));
        std::auto_ptr<ExternalTransaction> trans( new ExternalBillingTransaction(billId) );
        s->addTransaction(buf,trans);

        // adding operations
        DummyScagCommand cmd1(SMPP,111,33);
        s->createOperation(cmd1,CO_SUBMIT);
        DummyScagCommand cmd2(SMPP,112,34);
        s->createOperation(cmd2,CO_USSD_DIALOG);

        // adding properties
        SessionPropertyScope* scope;
        scope = s->getGlobalScope();
        scope->getProperty("global1");

        scope = s->getServiceScope(111);
        scope->getProperty("service1");

        int scopeId = s->createContextScope();
        scope = s->getContextScope(scopeId);
        scope->getProperty("context1");
        scope->getProperty("context2");

        scope = s->getOperationScope();
        scope->getProperty("operation1");
        scope->getProperty("operation2");

        s->waitAtLeast(200);
    }

    {
        smsc_log_debug(logg,"session is created:");
        scag_plog_debug(pl,logg);
        s->print(pl);
    }

    Serializer::Buf buffer;
    {
        smsc_log_debug(logg,"serialization");
        Serializer ser(buffer);
        ser << *s.get();
    }

    {
        HexDump hd;
        HexDump::string_type dump;
        hd.hexdump(dump,&buffer[0],buffer.size());
        hd.strdump(dump,&buffer[0],buffer.size());
        smsc_log_debug(logg,"session is serialized: sz=%u %s",
                       buffer.size(),hd.c_str(dump));
    }

    std::auto_ptr< Session > s2;
    {
        smsc_log_debug(logg,"deserializing");
        Deserializer dsr(buffer);
        s2.reset(new Session(sk));
        dsr >> *s2.get();
    }

    {
        smsc_log_debug(logg,"session is restored:");
        scag_plog_debug(pl,logg);
        s2->print(pl);
    }
    return 0;
}
