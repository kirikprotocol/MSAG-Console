#include "BillingManager.h"
#include "logger/Logger.h"
#include "scag/config/impl/ConfigManager2.h"

using smsc::logger::Logger;
using scag2::bill::BillingManagerImpl;
using scag2::bill::billid_type;
using scag2::config::ConfigManagerImpl;

namespace scag2 {
namespace bill {
class BillingManagerImplTester
{
public:
    BillingManagerImplTester( BillingManagerImpl* bm ) : bm_(bm) {}
    billid_type genBillId() {
        return bm_->genBillId();
    }

    void logEvent( billid_type bt ) {
        BillingInfoStruct bis;
        bm_->logEvent("open", true, bis, bt );
    }

private:
    BillingManagerImpl* bm_;
};
}
}

int main()
{
    Logger::initForTest(Logger::LEVEL_DEBUG);
    Logger* logMain = Logger::getInstance("main");

    smsc_log_info(logMain,"started");

    // create config manager, needed for bm
    {
        ConfigManagerImpl* cfg = new ConfigManagerImpl;
        cfg->Init();
    }

    // NOTE: we dont destroy bm
    scag2::bill::BillingManagerImplTester bm(new BillingManagerImpl);
    smsc_log_info(logMain,"billing manager created");

    for ( int i = 0; i < 10000; ++i ) {
        billid_type billId = bm.genBillId();
        // smsc_log_info(logMain,"billId=%lld",static_cast<long long>(billId));
        bm.logEvent(billId);
    }
    return 0;
}
