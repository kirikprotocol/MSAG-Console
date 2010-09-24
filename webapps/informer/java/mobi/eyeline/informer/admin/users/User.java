package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.Set;
import java.util.TreeSet;

/**
 * Настройки пользователя
 * @author Aleksandr Khalitov
 */
public class User implements Serializable{

  private String login;
  private String password;
  private Set<String> roles = new TreeSet<String>();
  private String firstName;
  private String lastName;
  private String dept;
  private String workPhone;
  private String homePhone;
  private String cellPhone;
  private String email;

  private UserPreferences prefs = new UserPreferences();

  private final ValidationHelper vh = new ValidationHelper(User.class);

  public User() {
  }

  public User(User user) {
    this.login = user.login;
    this.password = user.password;
    this.roles = user.getRoles() == null ? null : new TreeSet<String>(user.getRoles());
    this.firstName = user.firstName;
    this.lastName = user.lastName;
    this.dept = user.dept;
    this.workPhone = user.workPhone;
    this.homePhone = user.homePhone;
    this.cellPhone = user.cellPhone;
    this.email = user.email;
    this.prefs = user.prefs != null ? new UserPreferences(user.prefs) : null;
  }

  public UserPreferences getPrefs() {
    return prefs;
  }

  public void setPrefs(UserPreferences prefs) {
    this.prefs = prefs;
  }

  public String getLogin() {
    return login;
  }

  public void setLogin(String login) throws AdminException {
    vh.checkNotEmpty("login", login);
    this.login = login;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) throws AdminException{
    vh.checkNotEmpty("password", password);
    this.password = password;
  }

  public Set<String> getRoles() {
    return roles;
  }

  public void setRoles(Set<String> roles) {
    this.roles = roles;
  }

  public String getFirstName() {
    return firstName;
  }

  public void setFirstName(String firstName) throws AdminException {
    vh.checkNotEmpty("firstName", firstName);
    this.firstName = firstName;
  }

  public String getLastName() {
    return lastName;
  }

  public void setLastName(String lastName) throws AdminException {
    vh.checkNotEmpty("lastName", lastName);
    this.lastName = lastName;
  }

  public String getDept() {
    return dept;
  }

  public void setDept(String dept) throws AdminException {
    vh.checkNotEmpty("dept", dept);
    this.dept = dept;
  }

  public String getWorkPhone() {
    return workPhone;
  }

  public void setWorkPhone(String workPhone) throws AdminException {
    vh.checkNotEmpty("workPhone", workPhone);
    this.workPhone = workPhone;
  }

  public String getHomePhone() {
    return homePhone;
  }

  public void setHomePhone(String homePhone) throws AdminException {
    vh.checkNotEmpty("homePhone", homePhone);
    this.homePhone = homePhone;
  }

  public String getCellPhone() {
    return cellPhone;
  }

  public void setCellPhone(String cellPhone) throws AdminException {
    vh.checkNotEmpty("cellPhone", cellPhone);
    this.cellPhone = cellPhone;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) throws AdminException {
    vh.checkNotEmpty("email", email);
    this.email = email;
  }

  public boolean hasRole(String name) {
    return roles.contains(name);
  }
}
