package mobi.eyeline.informer.web.auth;

/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:38:32
 */

import java.util.Set;

public final class SmscPrincipal {
  private String name;
  private String password;
  private Set<String> roles;

  SmscPrincipal(String name, String password, Set<String> roles) {
    this.name = name;
    this.password = password;
    this.roles = roles;
  }

  /**
   * Returns the name of this principal.
   *
   * @return the name of this principal.
   */
  public String getName() {
    return name;
  }

  protected void setName(String name) {
    this.name = name;
  }

  protected String getPassword() {
    return password;
  }

  protected Set<String> getRoles() {
    return roles;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("SmscPrincipal");
    sb.append("{name='").append(name).append('\'');
    sb.append(", password='").append(password).append('\'');
    sb.append(", roles=").append(roles);
    sb.append('}');
    return sb.toString();
  }
}