package ru.sibinco.smppgw.beans.gw.users;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.beans.*;

import java.util.Map;


/**
 * Created by igork Date: 10.03.2004 Time: 17:55:16
 */
public class Edit extends EditBean
{
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


  public String getId()
  {
    return login;
  }

  protected void save() throws SmppgwJspException
  {
    if ((null != password && 0 < password.length()) || (null != confirmPassword && 0 < confirmPassword.length()))
      if (null != password ? !password.equals(confirmPassword) : !confirmPassword.equals(password))
        throw new SmppgwJspException(Constants.errors.users.PASSWORD_NOT_CONFIRM);

    final Map users = appContext.getUserManager().getUsers();

    if (isAdd()) {
      if (null == login || 0 == login.length())
        throw new SmppgwJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
      if (null == password || 0 == password.length())
        throw new SmppgwJspException(Constants.errors.users.PASSWORD_NOT_SPECIFIED);
      if (users.containsKey(login))
        throw new SmppgwJspException(Constants.errors.users.USER_ALREADY_EXISTS, login);
    } else {
      if (null == getEditId() || 0 == getEditId().length() || null == login || 0 == login.length())
        throw new SmppgwJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
      if (users.containsKey(login) && !getEditId().equals(login))
        throw new SmppgwJspException(Constants.errors.users.USER_ALREADY_EXISTS, login);

      final User user = (User) users.remove(getEditId());
      if (null != user) {
        if (null == password || 0 == password.length())
          password = user.getPassword();
      }
    }
    users.put(login, new User(login, password, roles, firstName, lastName, dept, workPhone, homePhone, cellPhone, email));
    appContext.getStatuses().setUsersChanged(true);
    throw new DoneException();
  }

  protected void load(final String userLogin) throws SmppgwJspException
  {
    if (null == userLogin || 0 == userLogin.length())
      throw new SmppgwJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);

    if (!appContext.getUserManager().getUsers().containsKey(userLogin))
      throw new SmppgwJspException(Constants.errors.users.USER_NOT_FOUND, userLogin);

    final User user = (User) appContext.getUserManager().getUsers().get(userLogin);
    login = user.getLogin();
    roles = (String[]) user.getRoles().toArray(new String[0]);
    firstName = user.getFirstName();
    lastName = user.getLastName();
    dept = user.getDept();
    workPhone = user.getWorkPhone();
    homePhone = user.getHomePhone();
    cellPhone = user.getCellPhone();
    email = user.getEmail();
  }


  public String getLogin()
  {
    return login;
  }

  public void setLogin(final String login)
  {
    this.login = login;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(final String password)
  {
    this.password = password;
  }

  public String getConfirmPassword()
  {
    return confirmPassword;
  }

  public void setConfirmPassword(final String confirmPassword)
  {
    this.confirmPassword = confirmPassword;
  }

  public String[] getRoles()
  {
    return roles;
  }

  public void setRoles(final String[] roles)
  {
    this.roles = roles;
  }

  public String getFirstName()
  {
    return firstName;
  }

  public void setFirstName(final String firstName)
  {
    this.firstName = firstName;
  }

  public String getLastName()
  {
    return lastName;
  }

  public void setLastName(final String lastName)
  {
    this.lastName = lastName;
  }

  public String getDept()
  {
    return dept;
  }

  public void setDept(final String dept)
  {
    this.dept = dept;
  }

  public String getWorkPhone()
  {
    return workPhone;
  }

  public void setWorkPhone(final String workPhone)
  {
    this.workPhone = workPhone;
  }

  public String getHomePhone()
  {
    return homePhone;
  }

  public void setHomePhone(final String homePhone)
  {
    this.homePhone = homePhone;
  }

  public String getCellPhone()
  {
    return cellPhone;
  }

  public void setCellPhone(final String cellPhone)
  {
    this.cellPhone = cellPhone;
  }

  public String getEmail()
  {
    return email;
  }

  public void setEmail(final String email)
  {
    this.email = email;
  }
}
