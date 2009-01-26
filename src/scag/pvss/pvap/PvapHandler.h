#ifndef _SCAG_PVSS_PVAP_PVAPHANDLER_H_
#define _SCAG_PVSS_PVAP_PVAPHANDLER_H_

namespace scag {
namespace pvss {
namespace pvap {

class PC_DEL;
class PC_DEL_RESP;
class PC_SET;
class PC_SET_RESP;
class PC_GET;
class PC_GET_RESP;
class PC_INC;
class PC_INC_RESP;
class PC_INC_MOD;
class PC_INC_MOD_RESP;
class PC_PING;
class PC_PING_RESP;
class PC_AUTH;
class PC_AUTH_RESP;
class PC_BATCH;
class PC_BATCH_RESP;

class PvapHandler
{
public:
    virtual ~PvapHandler() {}
    virtual void handle(std::auto_ptr<PC_DEL> msg) = 0;
    virtual void handle(std::auto_ptr<PC_DEL_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_SET> msg) = 0;
    virtual void handle(std::auto_ptr<PC_SET_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_GET> msg) = 0;
    virtual void handle(std::auto_ptr<PC_GET_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_INC> msg) = 0;
    virtual void handle(std::auto_ptr<PC_INC_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_INC_MOD> msg) = 0;
    virtual void handle(std::auto_ptr<PC_INC_MOD_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_PING> msg) = 0;
    virtual void handle(std::auto_ptr<PC_PING_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_AUTH> msg) = 0;
    virtual void handle(std::auto_ptr<PC_AUTH_RESP> msg) = 0;
    virtual void handle(std::auto_ptr<PC_BATCH> msg) = 0;
    virtual void handle(std::auto_ptr<PC_BATCH_RESP> msg) = 0;
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
