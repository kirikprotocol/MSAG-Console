package ru.novosoft.smsc.jsp.smsc.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;


/**
 * Created by igork
 * Date: 10.06.2004
 * Time: 15:55:32
 */
public class Index extends SmscBean
{
  public static final int RESULT_ADD_ACL = PageBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT_ACL = PageBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 2;

  private AclManager aclManager;
  private List aclNames;
  private AclInfo aclInfo = null;
  private List addresses = null;

  private String mbLookup = null;
  private String mbAddAcl = null;
  private String mbDelAcl = null;
  private String mbEditAcl = null;
  private String mbLookupAddresses = null;
  private String mbSave = null;

  private long aclId;
  private long lookupedAcl = -1;
  private String prefix = "";
  private String[] new_address = new String[0];
  private String[] deleted_address = new String[0];
  private String name = null;
  private String description = null;
  private char cache_type = AclInfo.ACT_UNDEFINED;


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    aclManager = appContext.getAclManager();
    try {
      aclNames = aclManager.getAclNames();
    } catch (AdminException e) {
      aclNames = new LinkedList();
      logger.debug("Couldn't get ACL names", e);
      return error(SMSCErrors.error.acl.COULDNT_GET_ACL_NAMES, e);
    }

    if (lookupedAcl != -1)
      lookup(lookupedAcl);

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbLookup != null)
      return lookup();
    if (mbAddAcl != null)
      return addAcl();
    if (mbDelAcl != null)
      return deleteAcl();
    if (mbEditAcl != null)
      return RESULT_EDIT_ACL;

    if (mbLookupAddresses != null)
      return lookupAddresses();

    if (mbSave != null)
      return save();

    return RESULT_OK;
  }

  private int save()
  {
    try {
      aclManager.updateAclInfo(lookupedAcl, name, description, cache_type);
      if (deleted_address != null && deleted_address.length > 0)
        aclManager.removeAddresses(lookupedAcl, Arrays.asList(deleted_address));
      if (new_address != null && new_address.length > 0)
        aclManager.addAddresses(lookupedAcl, Arrays.asList(new_address));
    } catch (AdminException e) {
      logger.debug("Couldn't remove or add acl addresses [id=" + lookupedAcl + "]", e);
      return error(SMSCErrors.error.acl.COULDNT_REMOVE_ADD_ADDRESSES, String.valueOf(lookupedAcl), e);
    }
    return RESULT_DONE;
  }

  private int lookupAddresses()
  {
    try {
      addresses = aclManager.lookupAddresses(lookupedAcl, prefix);
      return RESULT_OK;
    } catch (AdminException e) {
      logger.debug("Couldn't lookup acl addresses [id=" + lookupedAcl + "]", e);
      return error(SMSCErrors.error.acl.COULDNT_LOOKUP_ADDRESSES, String.valueOf(lookupedAcl), e);
    }
  }

  private int addAcl()
  {
    return RESULT_ADD_ACL;
  }

  private int deleteAcl()
  {
    try {
      aclManager.deleteAcl(aclId);
      return RESULT_DONE;
    } catch (AdminException e) {
      logger.debug("Couldn't delete acl [id=" + aclId + "]", e);
      return error(SMSCErrors.error.acl.COULDNT_DELETE_ACL, String.valueOf(aclId), e);
    }
  }

  private int lookup()
  {
    return lookup(aclId);
  }

  private int lookup(final long aclId)
  {
    try {
      aclInfo = aclManager.getAclInfo(aclId);
    } catch (AdminException e) {
      logger.debug("Couldn't lookup aclInfo [id=" + aclId + "]", e);
      return error(SMSCErrors.error.acl.COULDNT_LOOKUP_ACL, String.valueOf(aclId), e);
    }
    return RESULT_OK;
  }


  public List getAclNames()
  {
    return aclNames;
  }

  public AclInfo getAclInfo()
  {
    return aclInfo;
  }

  public List getAddresses()
  {
    return addresses;
  }


  public String getMbLookup()
  {
    return mbLookup;
  }

  public void setMbLookup(String mbLookup)
  {
    this.mbLookup = mbLookup;
  }

  public long getAclId()
  {
    return aclId;
  }

  public void setAclId(long aclId)
  {
    this.aclId = aclId;
  }

  public String getMbAddAcl()
  {
    return mbAddAcl;
  }

  public void setMbAddAcl(String mbAddAcl)
  {
    this.mbAddAcl = mbAddAcl;
  }

  public String getMbDelAcl()
  {
    return mbDelAcl;
  }

  public void setMbDelAcl(String mbDelAcl)
  {
    this.mbDelAcl = mbDelAcl;
  }

  public String getMbEditAcl()
  {
    return mbEditAcl;
  }

  public void setMbEditAcl(String mbEditAcl)
  {
    this.mbEditAcl = mbEditAcl;
  }

  public String getMbLookupAddresses()
  {
    return mbLookupAddresses;
  }

  public void setMbLookupAddresses(String mbLookupAddresses)
  {
    this.mbLookupAddresses = mbLookupAddresses;
  }

  public long getLookupedAcl()
  {
    return lookupedAcl;
  }

  public void setLookupedAcl(long lookupedAcl)
  {
    this.lookupedAcl = lookupedAcl;
  }

  public String getPrefix()
  {
    return prefix;
  }

  public void setPrefix(String prefix)
  {
    this.prefix = prefix;
  }

  public String[] getNew_address()
  {
    return new_address;
  }

  public void setNew_address(String[] new_address)
  {
    this.new_address = new_address;
  }

  public String[] getDeleted_address()
  {
    return deleted_address;
  }

  public void setDeleted_address(String[] deleted_address)
  {
    this.deleted_address = deleted_address;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
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

  public char getCache_type()
  {
    return cache_type;
  }

  public void setCache_type(char cache_type)
  {
    this.cache_type = cache_type;
  }
}
