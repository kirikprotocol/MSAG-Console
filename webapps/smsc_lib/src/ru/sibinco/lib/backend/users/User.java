package ru.sibinco.lib.backend.users;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;


/**
 * Created by igork Date: 03.03.2004 Time: 19:23:59
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
  private long providerId;


  public User(final String login, final String password, final String[] roles, final String firstName, final String lastName, final String dept,
              final String workPhone, final String homePhone, final String cellPhone, final String email, final long providerId)
  {
    this.login = login;
    this.password = password;
    this.roles = new TreeSet(Arrays.asList(null == roles ? new String[0] : roles));
    this.firstName = firstName;
    this.lastName = lastName;
    this.dept = dept;
    this.workPhone = workPhone;
    this.homePhone = homePhone;
    this.cellPhone = cellPhone;
    this.email = email;
    this.providerId = providerId;

    if (null == this.firstName) this.firstName = "";
    if (null == this.lastName) this.lastName = "";
    if (null == this.dept) this.dept = "";
    if (null == this.workPhone) this.workPhone = "";
    if (null == this.homePhone) this.homePhone = "";
    if (null == this.cellPhone) this.cellPhone = "";
    if (null == this.email) this.email = "";
  }

  public User(final Element userElem)
  {
    this.login = userElem.getAttribute("login");
    this.password = userElem.getAttribute("password");

    this.roles = new TreeSet();
    final NodeList roleList = userElem.getElementsByTagName("role");
    for (int i = 0; i < roleList.getLength(); i++) {
      final Element roleElem = (Element) roleList.item(i);
      this.roles.add(roleElem.getAttribute("name"));
    }

    this.firstName = userElem.getAttribute("firstName");
    this.lastName = userElem.getAttribute("lastName");
    this.dept = userElem.getAttribute("dept");
    this.workPhone = userElem.getAttribute("workPhone");
    this.homePhone = userElem.getAttribute("homePhone");
    this.cellPhone = userElem.getAttribute("cellPhone");
    this.email = userElem.getAttribute("email");
    try {
      this.providerId = Long.decode(userElem.getAttribute("providerId")).longValue();
    } catch (NumberFormatException e) {
      this.providerId = -1;
    }

    if (null == this.firstName) this.firstName = "";
    if (null == this.lastName) this.lastName = "";
    if (null == this.dept) this.dept = "";
    if (null == this.workPhone) this.workPhone = "";
    if (null == this.homePhone) this.homePhone = "";
    if (null == this.cellPhone) this.cellPhone = "";
    if (null == this.email) this.email = "";
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
  public boolean equals(final Object o)
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
    result += " providerId=\"" + providerId + '"';
    result += ">\n";
    for (Iterator i = roles.iterator(); i.hasNext();) {
      final String role = (String) i.next();
      result += "\t\t<role name=\"" + StringEncoderDecoder.encode(role) + "\"/>\n";
    }
    result += "\t</user>\n";

    return result;
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

  public Collection getRoles()
  {
    return roles;
  }

  public void setRoles(final Collection roles)
  {
    this.roles = null == roles ? new TreeSet() : new TreeSet(roles);
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

  public void grantRole(final String roleName)
  {
    roles.add(roleName);
  }

  public void revokeRole(final String roleName)
  {
    roles.remove(roleName);
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(final long providerId)
  {
    this.providerId = providerId;
  }
}

