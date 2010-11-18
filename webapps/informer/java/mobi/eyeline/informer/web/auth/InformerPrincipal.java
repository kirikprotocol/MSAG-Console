package mobi.eyeline.informer.web.auth;


import java.util.Set;

/**
 * Информациях о правах доступа пользователя
 *
 * @author Aleksandr Khalitov
 */
public final class InformerPrincipal {
  private String name;
  private final String password;
  private final Set<String> roles;

  public InformerPrincipal(String name, String password, Set<String> roles) {
    this.name = name;
    this.password = password;
    this.roles = roles;
  }

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