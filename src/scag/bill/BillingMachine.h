#ifndef SCAG_BILLING_MACHINE
#define SCAG_BILLING_MACHINE

#include <scag/re/actions/ActionFactory.h>

#include "Bill.h"

             
namespace scag { namespace bill {

    using scag::re::actions::ActionFactory;

    /**
     * Abstract class for user's BillingMachine implementation.
     * Concrete implementation should be placed in dynamic module (*.so) 
     */

    class Transaction
    {
       
    };


    class BillingMachine
    {
    protected:

        uint8_t     machine_id;

        BillingMachine() {};

    public:
        
        virtual ~BillingMachine() {};
        
        virtual void rollback(const Bill& bill) = 0;
        virtual ActionFactory * getActionFactory() const  = 0;
        
        inline void setMachineId(uint8_t id) { machine_id = id;}
    };

}}

typedef scag::bill::BillingMachine* (*initBillingMachineFn)(uint8_t machine_id, const std::string& cfg_dir);

#endif // SCAG_BILLING_MACHINE

