package ru.novosoft.smsc.web.config.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WMscManager implements MscManager {

  private final MscManager wrapped;
  private final Journal j;
  private final String user;

  public WMscManager(MscManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public Collection<Address> mscs() throws AdminException {
    return wrapped.mscs();
  }

  public void addMsc(Address msc) throws AdminException {
    wrapped.addMsc(msc);
    j.user(user).add().msc(msc.getSimpleAddress());
  }

  public void removeMsc(Address msc) throws AdminException {
    wrapped.removeMsc(msc);
    j.user(user).remove().msc(msc.getSimpleAddress());
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
