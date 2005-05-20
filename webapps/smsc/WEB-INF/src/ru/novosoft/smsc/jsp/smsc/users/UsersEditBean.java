package ru.novosoft.smsc.jsp.smsc.users;

/**
 * Created by igork
 * Date: Dec 2, 2002
 * Time: 4:59:33 PM
 */

import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.xml.WebXml;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public abstract class UsersEditBean extends SmscBean
{
	protected UserManager userManager = null;
	protected String login = null;
	protected String password = null;
	protected String confirmPassword = null;
	protected String[] roles = null;
	protected String firstName = null;
	protected String lastName = null;
	protected String dept = null;
	protected String workPhone = null;
	protected String homePhone = null;
	protected String cellPhone = null;
	protected String email = null;
	protected String mbCancel = null;
	protected String mbSave = null;
	protected Set rolesSet = new HashSet();
	protected List serviceRoles = new LinkedList();
	protected String[] prefsNames = null;
	protected String[] prefsValues = null;

	protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    userManager = appContext.getUserManager();
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
      return save();

    return RESULT_OK;
  }

  protected abstract int save();

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
  public String getLogin()
  {
    return login;
  }

  public void setLogin(String login)
  {
    this.login = login;
  }

  public void setPassword(String password)
  {
    this.password = password;
  }

  public void setConfirmPassword(String confirmPassword)
  {
    this.confirmPassword = confirmPassword;
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

  public String getFirstName()
  {
    return firstName;
  }

  public void setFirstName(String firstName)
  {
    this.firstName = firstName;
  }

  public String getLastName()
  {
    return lastName;
  }

  public void setLastName(String lastName)
  {
    this.lastName = lastName;
  }

  public String getDept()
  {
    return dept;
  }

  public void setDept(String dept)
  {
    this.dept = dept;
  }

  public String getWorkPhone()
  {
    return workPhone;
  }

  public void setWorkPhone(String workPhone)
  {
    this.workPhone = workPhone;
  }

  public String getHomePhone()
  {
    return homePhone;
  }

  public void setHomePhone(String homePhone)
  {
    this.homePhone = homePhone;
  }

  public String getCellPhone()
  {
    return cellPhone;
  }

  public void setCellPhone(String cellPhone)
  {
    this.cellPhone = cellPhone;
  }

  public String getEmail()
  {
    return email;
  }

  public void setEmail(String email)
  {
    this.email = email;
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

	public String[] getPrefsNames()
	{
		return prefsNames;
	}

	public void setPrefsNames(String[] names)
	{
		this.prefsNames = names;
	}

	public String[] getPrefsValues()
	{
		return prefsValues;
	}

	public void setPrefsValues(String[] values)
	{
		this.prefsValues = values;
	}

	public String getPref(String prefName)
	{
		String result = "";
		for (int i = 0; i < prefsNames.length; i++)
		{
			if (prefsNames[i].equals(prefName)) {result = prefsValues[i];break;}
		}
		return result;
	}
}
