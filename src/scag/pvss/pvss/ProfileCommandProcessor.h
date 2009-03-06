#ifndef _SCAG_PVSS_SERVER_PROFILECOMMANDPROCESSOR_H_
#define _SCAG_PVSS_SERVER_PROFILECOMMANDPROCESSOR_H_

#include <memory>

#include "scag/pvss/profile/Profile.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class BatchResponseComponent;

class ProfileCommandProcessor : public ProfileCommandVisitor {
public:
  ProfileCommandProcessor():rollback_(false), profile_(0), response_(0) {};
  bool visitBatchCommand(BatchCommand &cmd) throw(PvapException);
  bool visitDelCommand(DelCommand &cmd) throw(PvapException);
  bool visitGetCommand(GetCommand &cmd) throw(PvapException);
  bool visitIncCommand(IncCommand &cmd) throw(PvapException);
  bool visitIncModCommand(IncModCommand &cmd) throw(PvapException);
  bool visitSetCommand(SetCommand &cmd) throw(PvapException);
  void setProfile(Profile *pf);
  BatchResponseComponent* getBatchResponseComponent();
  Response* getResponse();
  bool rollback() const { return rollback_; };

private:
  Response::StatusType incModProperty(Property* property, uint32_t mod, uint32_t &result);

private:
  bool rollback_;
  Profile* profile_;
  std::auto_ptr<Response> response_;
};

}//pvss
}//scag2

#endif


