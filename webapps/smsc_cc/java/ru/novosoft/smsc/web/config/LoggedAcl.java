package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;

/**
 * @author Artem Snopkov
 */
class LoggedAcl implements Acl {

  private final Acl wrappedValue;
  private final Journal j;
  private final String user;

  LoggedAcl(Acl wrappedValue, Journal j, String user) {
    this.wrappedValue = wrappedValue;
    this.j = j;
    this.user = user;
  }

  public void updateInfo(String name, String description) throws AdminException {
    String oldName = wrappedValue.getName();
    String oldDesc = wrappedValue.getDescription();
    wrappedValue.updateInfo(name, description);
    j.logAclInfoChanged(oldName, oldDesc, name, description, user);
  }

  public List<Address> getAddresses() throws AdminException {
    return wrappedValue.getAddresses();
  }

  public void addAddresses(List<Address> addresses) throws AdminException {
    wrappedValue.addAddresses(addresses);
    j.logAddAddressesToAcl(wrappedValue.getId(), wrappedValue.getName(), addresses, user);
  }

  public void removeAddresses(List<Address> addresses) throws AdminException {
    wrappedValue.removeAddresses(addresses);
    j.logRemoveAddressesFromAcl(wrappedValue.getId(), wrappedValue.getName(), addresses, user);
  }

  public int getId() {
    return wrappedValue.getId();
  }

  public String getName() {
    return wrappedValue.getName();
  }

  public String getDescription() {
    return wrappedValue.getDescription();
  }
}
