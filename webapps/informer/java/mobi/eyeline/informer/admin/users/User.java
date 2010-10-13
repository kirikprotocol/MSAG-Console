package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.io.Serializable;
import java.util.Locale;
import java.util.Set;
import java.util.TreeSet;

/**
 * Настройки пользователя
 * @author Aleksandr Khalitov
 */
public class User implements Serializable{

  private String login;
  private String password;
  private Status status;
  private String firstName;
  private String lastName;
  private String phone;
  private String email;
  private String organization;
  private Locale locale;
  private Set<String> roles = new TreeSet<String>();




  private final ValidationHelper vh = new ValidationHelper(User.class);



  public User() {
  }

  public User(User user) {
    this.login = user.login;
    this.password = user.password;
    this.roles = user.getRoles() == null ? null : new TreeSet<String>(user.getRoles());
    this.firstName = user.firstName;
    this.lastName = user.lastName;
    this.phone = user.phone;
    this.email = user.email;
    this.status = user.status;
    this.organization=user.organization;
    this.locale = user.locale == null ? null : new Locale(user.locale.getLanguage());
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


  public String getPhone() {
    return phone;
  }

  public void setPhone(String phone) throws AdminException {
    vh.checkNotEmpty("phone", phone);
    this.phone = phone;
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

  public void setStatus(Status status) throws AdminException {
    vh.checkNotNull("status", status);
    this.status = status;
  }

  public Status getStatus() {
    return status;
  }

  public void setOrganization(String organization) {
    this.organization = organization;
  }

  public String getOrganization() {
    return organization;
  }

  public void setLocale(Locale locale) {
      this.locale = locale;
  }

  public Locale getLocale() {
    return locale;
  }

  public enum Status {
    ENABLED,
    DISABLED
  }
}
