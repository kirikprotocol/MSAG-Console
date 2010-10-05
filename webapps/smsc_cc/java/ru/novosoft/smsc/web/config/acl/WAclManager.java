package ru.novosoft.smsc.web.config.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Декоратор для AclManager, добавляющий в него функции журналирования
 * @author Artem Snopkov
 */
public class WAclManager implements AclManager {

  private final AclManager wrapped;
  private final Journal j;
  private final String user;

  public WAclManager(AclManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public Acl createAcl(String name, String description, List<Address> addresses) throws AdminException {
    Acl acl = wrapped.createAcl(name, description, addresses);
    j.user(user).add().acl(acl.getId()+"", name, description);    
    return new WAcl(acl, j, user);
  }

  public void removeAcl(int aclId) throws AdminException {
    wrapped.removeAcl(aclId);
    j.user(user).remove().acl(aclId+"");
  }

  public Acl getAcl(int id) throws AdminException {
    Acl acl = wrapped.getAcl(id);
    if (acl == null)
      return null;
    return new WAcl(acl, j, user);
  }

  public List<Acl> acls() throws AdminException {
    List<Acl>  acls = wrapped.acls();
    List<Acl> result = new ArrayList<Acl>(acls.size());
    for (Acl acl : acls)
      result.add(new WAcl(acl, j, user));
    return result;
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
