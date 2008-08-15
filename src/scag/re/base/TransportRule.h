#ifndef _SCAG_RE_TRANSPORTRULE_H
#define _SCAG_RE_TRANSPORTRULE_H

#include "logger/Logger.h"
#include "RuleStatus2.h"

namespace scag2 {

namespace transport {
class SCAGCommand;
}

namespace sessions {
class Session;
}

namespace re {

class TransportRule
{
public:
    class Guard
    {
    public:
        Guard( TransportRule& tr,
               transport::SCAGCommand& cmd,
               sessions::Session& session,
               RuleStatus& rs ) :
        init_(false), tr_(tr), cmd_(cmd), sess_(session), rs_(rs) 
        {
        }

        void init() {
            if ( !init_ ) {
                tr_.setupSessionOperation( cmd_, sess_, rs_ );
                init_ = true;
            }
        }

        ~Guard() {
            if (init_) tr_.resetSessionOperation( cmd_, sess_, rs_ );
        }

    private:
        Guard();
        Guard( const Guard& g );
        Guard& operator = ( const Guard& g );
    private:
        bool                     init_;
        TransportRule&           tr_;
        transport::SCAGCommand&  cmd_;
        sessions::Session&       sess_;
        RuleStatus&              rs_;
    };
    virtual ~TransportRule() {}

protected:
    TransportRule();
    virtual void setupSessionOperation( transport::SCAGCommand& command,
                                        sessions::Session& session,
                                        RuleStatus& rs ) = 0;
    virtual void resetSessionOperation( transport::SCAGCommand& command,
                                        sessions::Session& session,
                                        RuleStatus& rs ) = 0;

protected:
    static smsc::logger::Logger* log_;
};

} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_TRANSPORTRULE_H */
