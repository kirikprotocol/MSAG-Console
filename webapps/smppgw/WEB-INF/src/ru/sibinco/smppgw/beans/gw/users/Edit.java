package ru.sibinco.smppgw.beans.gw.users;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.beans.*;

import java.util.Map;


/**
 * Created by igork
 * Date: 10.03.2004
 * Time: 17:55:16
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
    if ((password != null && password.length() > 0) || (confirmPassword != null && confirmPassword.length() > 0))
      if (password != null ? !password.equals(confirmPassword) : !confirmPassword.equals(password))
        throw new SmppgwJspException(Constants.errors.users.PASSWORD_NOT_CONFIRM);

    final Map users = appContext.getUserManager().getUsers();

    if (isAdd()) {
      if (login == null || login.length() == 0)
        throw new SmppgwJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
      if (password == null || password.length() == 0)
        throw new SmppgwJspException(Constants.errors.users.PASSWORD_NOT_SPECIFIED);
      if (users.containsKey(login))
        throw new SmppgwJspException(Constants.errors.users.USER_ALREADY_EXISTS, login);
    } else {
      if (getEditId() == null || getEditId().length() == 0 || login == null || login.length() == 0)
        throw new SmppgwJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
      if (users.containsKey(login) && !getEditId().equals(login))
        throw new SmppgwJspException(Constants.errors.users.USER_ALREADY_EXISTS, login);

      User user = (User) users.remove(getEditId());
      if (user != null) {
        if (password == null || password.length() == 0)
          password = user.getPassword();
      }
    }
    users.put(login, new User(login, password, roles, firstName, lastName, dept, workPhone, homePhone, cellPhone, email));
    throw new DoneException();
  }

  protected void load(final String userLogin) throws SmppgwJspException
  {
    if (userLogin == null || userLogin.length() == 0)
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

  public void setLogin(String login)
  {
    this.login = login;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(String password)
  {
    this.password = password;
  }

  public String getConfirmPassword()
  {
    return confirmPassword;
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
    this.roles = roles;
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
}
