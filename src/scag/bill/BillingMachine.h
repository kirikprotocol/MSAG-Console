#ifndef SCAG_BILLING_MACHINE
#define SCAG_BILLING_MACHINE

#include <scag/re/actions/ActionFactory.h>

#include "Bill.h"

namespace scag { namespace bill
{
    using scag::re::actions::ActionFactory;

    /**
     * Abstract class for user's BillingMachine implementation.
     * Concrete implementation should be placed in dynamic module (*.so) 
     */
    class BillingMachine
    {
    protected:

        uint8_t     machine_id;

        BillingMachine() {};

    public:
        
        virtual ~BillingMachine() {};
        
        /**
         * Function should rollback user's billing transaction
         */
        virtual void rollback(const Bill& bill) = 0;
        
        /**
         * Function should return pointer to user's billing actions factory
         */ 
        virtual const ActionFactory& getActionFactory() const  = 0;
        
        inline void setMachineId(uint8_t id) { // ???
            machine_id = id;
        }
    };

    /** TODO: Place this definition to dynamic module (*.so) header file.
     *  Implement initialization function in dynamic module (*.so).
     *  Function should init user's billing machine & return pointer to it.
     *
    extern "C" BillingMachine* initBillingMachine(uint8_t machine_id, const std::string& cfg_dir);
     */

}}

#endif // SCAG_BILLING_MACHINE

