#ifndef _SCAG_PVSS_PVAP_PVAPBCHANDLER_H_
#define _SCAG_PVSS_PVAP_PVAPBCHANDLER_H_

#include <memory>

namespace scag {
namespace pvss {
namespace pvap {

class BC_DEL;
class BC_DEL_RESP;
class BC_SET;
class BC_SET_RESP;
class BC_GET;
class BC_GET_RESP;
class BC_INC;
class BC_INC_RESP;
class BC_INC_MOD;
class BC_INC_MOD_RESP;

class PvapBCHandler
{
public:
    virtual ~PvapBCHandler() {}
    virtual void handle(std::auto_ptr<BC_DEL> msg) = 0;
    virtual void handle(std::auto_ptr<BC_DEL_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<BC_SET> msg) = 0;
    virtual void handle(std::auto_ptr<BC_SET_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<BC_GET> msg) = 0;
    virtual void handle(std::auto_ptr<BC_GET_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<BC_INC> msg) = 0;
    virtual void handle(std::auto_ptr<BC_INC_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<BC_INC_MOD> msg) = 0;
    virtual void handle(std::auto_ptr<BC_INC_MOD_RESP> msg) = 0;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

namespace scag2 {
namespace pvss {
namespace pvap = scag::pvss::pvap;
}
}

#endif /* !_SCAG_PVSS_PVAP_PVAPHANDLER_H_ */
