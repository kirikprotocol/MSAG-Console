#include "AclCommandListener.h"
#include <exception>

namespace smsc {
namespace cluster {

using namespace smsc::util;
using namespace smsc::core::buffers;

using smsc::acls::AclPhoneNumber;
using smsc::acls::AclCacheType;

AclCommandListener::AclCommandListener(AclAbstractMgr   *aclmgr_)
    : aclmgr(aclmgr_)
{
}

void AclCommandListener::handle(const Command& command)
{

	switch( command.getType() ){
	case ACLREMOVE_CMD:
		aclRemove(command);
		break;
    case ACLCREATE_CMD:
        aclCreate(command);
		break;
    case ACLUPDATEINFO_CMD:
        aclUpdateInfo(command);
		break;
    case ACLREMOVEADDRESSES_CMD:
        aclRemoveAddresses(command);
		break;
    case ACLADDADDRESSES_CMD:
        aclAddAddresses(command);
		break;
	}
}

void AclCommandListener::aclRemove(const Command& command)
{
    smsc::acls::AclIdent aclId;

    const AclRemoveCommand* cmd = dynamic_cast<const AclRemoveCommand*>(&command);

    cmd->getArgs(aclId);

    
    
    try {
      aclmgr->remove(aclId);
    } catch (std::exception &e) {
      throw Exception("Could not remove ACL, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw Exception("Could not remove ACL, nested: \"%s\"", e);
    } catch (...) {
      throw Exception("Could not remove ACL, nested: Unknown exception");
    }
  
}

void AclCommandListener::aclCreate(const Command& command)
{
    std::string name;
    std::string description;
    std::string cache_type;
    bool cache_type_present;
    File::offset_type offset;
    std::vector<std::string> phones;

    const AclCreateCommand* cmd = dynamic_cast<const AclCreateCommand*>(&command);

    cmd->getArgs(name, description, cache_type, cache_type_present, offset, phones);

    
    cache_type_present = cache_type.length() > 0;

    std::vector<AclPhoneNumber> phones_;
    for (std::vector<std::string>::iterator i = phones.begin(); i != phones.end(); i++)
    {
      std::string addrStr = *i;
      if (addrStr.length())
      {
        phones_.push_back(addrStr);
      }
    }

    try {
        if (cache_type_present)
            aclmgr->create2(name.c_str(), description.c_str(), phones_, (AclCacheType)(cache_type.c_str()[0]));
        else
            aclmgr->create2(name.c_str(), description.c_str(), phones_);
    } catch (std::exception &e) {
      throw Exception("Could not create ACL, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw Exception("Could not create ACL, nested: \"%s\"", e);
    } catch (...) {
      throw Exception("Could not create ACL, nested: Unknown exception");
    }
}

void AclCommandListener::aclUpdateInfo(const Command& command)
{
    smsc::acls::AclIdent aclId;
    std::string name;
    std::string description;
    std::string cache_type;

    const AclUpdateInfoCommand* cmd = dynamic_cast<const AclUpdateInfoCommand*>(&command);

    cmd->getArgs(aclId, name, description, cache_type);


    bool cache_type_present = cache_type.length() > 0;

    try {
      if (cache_type_present)
        aclmgr->updateAclInfo(aclId, name.c_str(), description.c_str(), (AclCacheType)(cache_type.c_str()[0]));
      else
        aclmgr->updateAclInfo(aclId, name.c_str(), description.c_str());
    } catch (std::exception &e) {
      throw Exception("Could not update ACL info, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw Exception("Could not update ACL info, nested: \"%s\"", e);
    } catch (...) {
      throw Exception("Could not update ACL info, nested: Unknown exception");
    }  
  
}

void AclCommandListener::aclRemoveAddresses(const Command& command)
{
    smsc::acls::AclIdent aclId;
    std::vector<std::string> addresses;

    const AclRemoveAddressesCommand* cmd = dynamic_cast<const AclRemoveAddressesCommand*>(&command);

    cmd->getArgs(aclId, addresses);

    try {
      
        for (std::vector<std::string>::const_iterator i = addresses.begin(); i != addresses.end(); i++) {
            aclmgr->removePhone(aclId, *i);
        }
    } catch (std::exception &e) {
      throw Exception("Could not remove addresses, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw Exception("Could not remove addresses, nested: \"%s\"", e);
    } catch (...) {
      throw Exception("Could not remove addresses, nested: Unknown exception");
    }
  
}

void AclCommandListener::aclAddAddresses(const Command& command)
{
    smsc::acls::AclIdent aclId;
    std::vector<std::string> addresses;

    const AclAddAddressesCommand* cmd = dynamic_cast<const AclAddAddressesCommand*>(&command);

    cmd->getArgs(aclId, addresses);

    try {
        for (std::vector<std::string>::const_iterator i = addresses.begin(); i != addresses.end(); i++) {
            aclmgr->addPhone(aclId, *i);
        }
    } catch (std::exception &e) {
      throw Exception("Could not add addresses, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw Exception("Could not add addresses, nested: \"%s\"", e);
    } catch (...) {
      throw Exception("Could not add addresses, nested: Unknown exception");
    }
  
}

}
}
