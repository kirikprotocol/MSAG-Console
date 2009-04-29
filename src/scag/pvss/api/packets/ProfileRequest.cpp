#include "ProfileRequest.h"
#include "ProfileCommandVisitor.h"
#include "BatchCommand.h"

namespace {
using namespace scag2::pvss;

class AbbrevCmdVisitor : public ProfileCommandVisitor
{
public:
    AbbrevCmdVisitor() {}
    void process( ProfileCommand& cmd ) {
        string_.reserve(20);
        cmd.visit(*this);
    }
    virtual bool visitDelCommand( DelCommand& ) {
        string_.push_back('d');
        return true;
    }
    virtual bool visitSetCommand( SetCommand& ) {
        string_.push_back('s');
        return true;
    }
    virtual bool visitGetCommand( GetCommand& ) {
        string_.push_back('g');
        return true;
    }
    virtual bool visitIncCommand( IncCommand& ) {
        string_.push_back('i');
        return true;
    }
    virtual bool visitIncModCommand( IncModCommand& ) {
        string_.push_back('I');
        return true;
    }
    virtual bool visitBatchCommand( BatchCommand& cmd ) {
        string_.push_back( cmd.isTransactional() ? 'b' : 'B' );
        string_.push_back('{');
        const std::vector< BatchRequestComponent* >& content = cmd.getBatchContent();
        for ( std::vector< BatchRequestComponent* >::const_iterator i = content.begin();
              i != content.end(); ++i ) {
            (*i)->visit(*this);
        }
        string_.push_back('}');
        return true;
    }
    const std::string& toString() const { return string_; }

private:
    std::string string_;
};

}

namespace scag2 {
namespace pvss {

ProfileRequest::~ProfileRequest()
{
    logDtor();
    if (timing_) {
        if ( !timing_->result.empty() && logtm_->isInfoEnabled() ) {
            AbbrevCmdVisitor acv;
            if (command_) { acv.process(*command_); }
            smsc_log_info(logtm_,"timing: %s %s",
                          timing_->result.c_str(),acv.toString().c_str());
        }
        delete timing_;
    }
    if (command_) {delete command_;}
}

} // namespace pvss
} // namespace scag2
