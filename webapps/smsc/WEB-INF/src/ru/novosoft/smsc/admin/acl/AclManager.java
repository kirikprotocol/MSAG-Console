package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;


/**
 * Created by igork
 * Date: 10.06.2004
 * Time: 15:58:37
 */
public class AclManager
{
  private final SMSCAppContext appContext;

  public AclManager(SMSCAppContext appContext)
  {
    this.appContext = appContext;
  }

  public AclInfo getAclInfo(long id) throws AdminException
  {
    return appContext.getSmsc().aclGetInfo(id);
  }

  public List getAclNames() throws AdminException
  {
    return appContext.getSmsc().aclListNames();
  }

  public void deleteAcl(long aclId) throws AdminException
  {
    appContext.getSmsc().aclRemove(aclId);
  }

  public void createAcl(String name, String description, List addresses, char cache_type) throws AdminException
  {
    appContext.getSmsc().aclCreate(name, description, addresses, cache_type);
  }

  public void updateAclInfo(long aclId, String name, String description, char cache_type) throws AdminException
  {
    appContext.getSmsc().aclUpdateInfo(aclId, name, description, cache_type);
  }

  public List lookupAddresses(long aclId, String addressPrefix) throws AdminException
  {
    return appContext.getSmsc().aclLookupAddresses(aclId, addressPrefix);
  }

  public void removeAddresses(long aclId, List addresses) throws AdminException
  {
    appContext.getSmsc().aclRemoveAddresses(aclId, addresses);
  }

  public void addAddresses(long aclId, List addresses) throws AdminException
  {
    appContext.getSmsc().aclAddAddresses(aclId, addresses);
  }

}
