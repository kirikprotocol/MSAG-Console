package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.util.List;

/**
 * @author Artem Snopkov
 */
class AclImpl implements Acl {
  static final int MAX_NAME_LEN = 31;
  static final int MAX_DESCRIPTION_LEN = 127;

  private static final transient ValidationHelper vh = new ValidationHelper(Acl.class);

  private final int id;
  private transient final AclManager manager;
  private String name;
  private String description;

  AclImpl(AclManager manager, int id, String name, String description) {
    this.id = id;
    this.manager = manager;
    this.name = name;
    this.description = description;
  }

  static void checkInfo(String name, String description) throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkLen("name", name, 0, MAX_NAME_LEN);
    if (description != null)
      vh.checkLen("description", description, 0, MAX_DESCRIPTION_LEN);
  }

  static void checkAddresses(List<Address> addresses) throws AdminException {
    vh.checkNoNulls("addresses", addresses);
  }


  public void updateInfo(String name, String description) throws AdminException {
    checkInfo(name, description);
    manager.updateAcl(id, name, description);
    this.name = name;
    this.description = description;
  }

  public List<Address> getAddresses() throws AdminException {
    return manager.getAddresses(id);
  }

  public void addAddresses(List<Address> addresses) throws AdminException {
    checkAddresses(addresses);
    manager.addAddresses(id, addresses);
  }

  public void removeAddresses(List<Address> addresses) throws AdminException {
    checkAddresses(addresses);
    manager.removeAddresses(id, addresses);
  }

  public int getId() {
    return id;
  }

  public String getName() {
    return name;
  }

  public String getDescription() {
    return description;
  }
}
