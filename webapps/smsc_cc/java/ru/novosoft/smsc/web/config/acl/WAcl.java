package ru.novosoft.smsc.web.config.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.List;

/**
 * Декоратор для Acl, добавляющий в него функции журналирования
 * @author Artem Snopkov
 */
class WAcl implements Acl {

  private final Acl wrappedValue;
  private final Journal j;
  private final String user;

  WAcl(Acl wrappedValue, Journal j, String user) {
    this.wrappedValue = wrappedValue;
    this.j = j;
    this.user = user;
  }

  public void updateInfo(String name, String description) throws AdminException {
    String oldName = wrappedValue.getName();
    String oldDesc = wrappedValue.getDescription();
    wrappedValue.updateInfo(name, description);
    if (!oldName.equals(name)) {
      JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.ACL, user);
      r.setDescription("property_changed", "name", oldName, name);
    }
    if (!oldDesc.equals(description)) {
      JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.ACL, user);
      r.setDescription("property_changed", "description", oldDesc, description);
    }
  }

  public List<Address> getAddresses() throws AdminException {
    return wrappedValue.getAddresses();
  }

  public void addAddresses(List<Address> addresses) throws AdminException {
    wrappedValue.addAddresses(addresses);
    if (!addresses.isEmpty()) {
      JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.ACL, user);
      StringBuilder sb = new StringBuilder();
      for (Address address : addresses) {
        if (sb.length() > 0)
          sb.append(", ");
        sb.append(address);
      }
      r.setDescription("acl.add.addresses", getName(), sb.toString());
    }
  }

  public void removeAddresses(List<Address> addresses) throws AdminException {
    wrappedValue.removeAddresses(addresses);
    if (!addresses.isEmpty()) {
      JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.ACL, user);
      StringBuilder sb = new StringBuilder();
      for (Address address : addresses) {
        if (sb.length() > 0)
          sb.append(", ");
        sb.append(address);
      }
      r.setDescription("acl.remove.addresses", getName(), sb.toString());
    }
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
