package ru.novosoft.smsc.jsp.smsc.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.List;


/**
 * Created by igork
 * Date: 22.06.2004
 * Time: 19:48:55
 */
public class Edit extends PageBean
{
  private AclManager aclManager;

  private boolean add = false;
  private long id = -1;
  private String name = "";
  private String description = "";
  private char cache_type = AclInfo.ACT_UNDEFINED;
  private String[] addresses = new String[0];

  private String mbSave = null;
  private String mbCancel = null;
  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    aclManager = appContext.getAclManager();
    if (!initialized && id != -1) {
      try {
        AclInfo acl = aclManager.getAclInfo(id);
        logger.debug("get AclInfo[" + acl.getId() + ", \"" + acl.getName() + ", \"" + acl.getDescription() + "\"]");
        name = acl.getName();
        description = acl.getDescription();
      } catch (AdminException e) {
        logger.error("Could not get info for acl #" + id, e);
        return error(SMSCErrors.error.acl.COULDNT_GET_ACL_INFO, String.valueOf(id), e);
      }
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return cancel();

    if (mbSave != null)
      return save();

    return RESULT_OK;
  }

  private int cancel()
  {
    return RESULT_DONE;
  }

  private int save()
  {
    if (add) {
      try {
        aclManager.createAcl(name, description, Arrays.asList(addresses), cache_type);
        return RESULT_DONE;
      } catch (AdminException e) {
        logger.error("Couldn't create acl " + id, e);
        return error(SMSCErrors.error.acl.COULDNT_CREATE_ACL, String.valueOf(id), e);
      }
    }
    else {
      try {
        aclManager.updateAclInfo(id, name, description, cache_type);
        return RESULT_DONE;
      } catch (AdminException e) {
        logger.error("Couldn't update info for acl #" + id, e);
        return error(SMSCErrors.error.acl.COULDNT_UPDATE_ACL_INFO, String.valueOf(id), e);
      }
    }
  }

  public boolean isAdd()
  {
    return add;
  }

  public void setAdd(boolean add)
  {
    this.add = add;
  }

  public long getId()
  {
    return id;
  }

  public void setId(long id)
  {
    this.id = id;
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String getDescription()
  {
    return description;
  }

  public void setDescription(String description)
  {
    this.description = description;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String[] getAddresses()
  {
    return addresses;
  }

  public void setAddresses(String[] addresses)
  {
    this.addresses = addresses;
  }

  public char getCache_type()
  {
    return cache_type;
  }

  public void setCache_type(char cache_type)
  {
    this.cache_type = cache_type;
  }
}
