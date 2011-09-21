package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpoint {

  private static final ValidationHelper vh = new ValidationHelper(SmppGWEndpoint.class);

  private String name;
  private String systemId;
  private String password;

  private boolean enabled;

  public SmppGWEndpoint() {
  }

  public SmppGWEndpoint(SmppGWEndpoint e) {
    this.name = e.name;
    this.systemId = e.systemId;
    this.password = e.password;
    this.enabled = e.enabled;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getSystemId() {
    return systemId;
  }

  public void setSystemId(String systemId) {
    this.systemId = systemId;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public void validate() throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkLen("systemId", systemId, 3, 15);
    vh.checkLen("password", password, 3, 8);
    vh.checkLatinDigit("systemId", systemId);
    vh.checkLatinDigit("password", password);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmppGWEndpoint that = (SmppGWEndpoint) o;

    if (name != null ? !name.equals(that.name) : that.name != null) return false;
    if (password != null ? !password.equals(that.password) : that.password != null) return false;
    if (systemId != null ? !systemId.equals(that.systemId) : that.systemId != null) return false;
    return enabled == that.enabled;
  }
}
