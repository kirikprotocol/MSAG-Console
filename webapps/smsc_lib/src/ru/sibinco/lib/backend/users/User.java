package ru.sibinco.lib.backend.users;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;


/**
 * Created by igork
 * Date: 03.03.2004
 * Time: 19:23:59
 */
public class User implements Principal
{
  public static final String[] columnNames = {
    "login", "password", "roles", "firstName", "lastName", "dept",
    "workPhone", "homePhone", "cellPhone", "email"
  };

  private String login = null;
  private String password = null;
  private Set roles = null;
  private String firstName = null;
  private String lastName = null;
  private String dept = null;
  private String workPhone = null;
  private String homePhone = null;
  private String cellPhone = null;
  private String email = null;


  public User(String login, String password, String[] roles, String firstName, String lastName, String dept, String workPhone, String homePhone, String cellPhone, String email)
  {
    this.login = login;
    this.password = password;
    this.roles = new TreeSet(Arrays.asList(roles == null ? new String[0] : roles));
    this.firstName = firstName;
    this.lastName = lastName;
    this.dept = dept;
    this.workPhone = workPhone;
    this.homePhone = homePhone;
    this.cellPhone = cellPhone;
    this.email = email;

    if (this.firstName == null) this.firstName = "";
    if (this.lastName == null) this.lastName = "";
    if (this.dept == null) this.dept = "";
    if (this.workPhone == null) this.workPhone = "";
    if (this.homePhone == null) this.homePhone = "";
    if (this.cellPhone == null) this.cellPhone = "";
    if (this.email == null) this.email = "";
  }

  public User(Element userElem)
  {
    this.login = userElem.getAttribute("login");
    this.password = userElem.getAttribute("password");

    this.roles = new TreeSet();
    NodeList roleList = userElem.getElementsByTagName("role");
    for (int i = 0; i < roleList.getLength(); i++) {
      Element roleElem = (Element) roleList.item(i);
      this.roles.add(roleElem.getAttribute("name"));
    }

    this.firstName = userElem.getAttribute("firstName");
    this.lastName = userElem.getAttribute("lastName");
    this.dept = userElem.getAttribute("dept");
    this.workPhone = userElem.getAttribute("workPhone");
    this.homePhone = userElem.getAttribute("homePhone");
    this.cellPhone = userElem.getAttribute("cellPhone");
    this.email = userElem.getAttribute("email");

    if (this.firstName == null) this.firstName = "";
    if (this.lastName == null) this.lastName = "";
    if (this.dept == null) this.dept = "";
    if (this.workPhone == null) this.workPhone = "";
    if (this.homePhone == null) this.homePhone = "";
    if (this.cellPhone == null) this.cellPhone = "";
    if (this.email == null) this.email = "";
  }

  public String getName()
  {
    return login;
  }

  public String getId()
  {
    return login;
  }

  /**
   * Compares only login field
   *
   * @param o object to compare
   * @return
   */
  public boolean equals(Object o)
  {
    if (this == o) return true;
    if (!(o instanceof User)) return false;

    final User user = (User) o;

    if (!login.equals(user.login)) return false;

    return true;
  }

  public int hashCode()
  {
    return login.hashCode();
  }

  public String getXmlText()
  {
    String result = "\t<user";
    result += " login=\"" + StringEncoderDecoder.encode(login) + '"';
    result += " password=\"" + StringEncoderDecoder.encode(password) + '"';
    result += " firstName=\"" + StringEncoderDecoder.encode(firstName) + '"';
    result += " lastName=\"" + StringEncoderDecoder.encode(lastName) + '"';
    result += " dept=\"" + StringEncoderDecoder.encode(dept) + '"';
    result += " workPhone=\"" + StringEncoderDecoder.encode(workPhone) + '"';
    result += " homePhone=\"" + StringEncoderDecoder.encode(homePhone) + '"';
    result += " cellPhone=\"" + StringEncoderDecoder.encode(cellPhone) + '"';
    result += " email=\"" + StringEncoderDecoder.encode(email) + '"';
    result += ">\n";
    for (Iterator i = roles.iterator(); i.hasNext();) {
      String role = (String) i.next();
      result += "\t\t<role name=\"" + StringEncoderDecoder.encode(role) + "\"/>\n";
    }
    result += "\t</user>\n";

    return result;
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

  public Collection getRoles()
  {
    return roles;
  }

  public void setRoles(Collection roles)
  {
    this.roles = roles == null ? new TreeSet() : new TreeSet(roles);
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

  public void grantRole(String roleName)
  {
    roles.add(roleName);
  }

  public void revokeRole(String roleName)
  {
    roles.remove(roleName);
  }
}

