package ru.novosoft.smsc.jsp.smsc.categories;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.xml.WebXml;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public abstract class CategoriesEditBean extends SmscBean
{
  protected CategoryManager categoryManager = null;
  protected String name = null;
  protected String id = null;
  protected String[] roles = null;
  protected String mbCancel = null;
  protected String mbSave = null;
  protected Set rolesSet = new HashSet();
  protected List serviceRoles = new LinkedList();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    categoryManager = appContext.getCategoryManager();
    serviceRoles = appContext.getWebXmlConfig().getRoleNames();
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    else if (mbSave != null)
      return save(request);

    return RESULT_OK;
  }

  protected abstract int save(HttpServletRequest request);

  public abstract boolean isNew();

  public boolean isUserInRole(String rolename)
  {
    return rolesSet.contains(rolename);
  }

  public String getServiceIdFromRole(String roleName)
  {
    return WebXml.getServiceIdFromRole(roleName);
  }

  /**
   * *************************************** properties ***************************************************
   */
  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String getId()
  {
    return id;
  }

  public void setId(String id)
  {
    this.id = id;
  }

  public String[] getRoles()
  {
    return roles;
  }

  public void setRoles(String[] roles)
  {
    if (roles == null)
      roles = new String[0];
    this.roles = roles;
    rolesSet.clear();
    rolesSet.addAll(Arrays.asList(roles));
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public List getServiceRoles()
  {
    return serviceRoles;
  }
}
