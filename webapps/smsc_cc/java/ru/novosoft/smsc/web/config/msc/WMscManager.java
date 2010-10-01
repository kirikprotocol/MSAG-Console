package ru.novosoft.smsc.web.config.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

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
    JournalRecord r = j.addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.MSC, user);
    r.setDescription("msc.added", msc.getSimpleAddress());
  }

  public void removeMsc(Address msc) throws AdminException {
    wrapped.removeMsc(msc);
    JournalRecord r = j.addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.MSC, user);
    r.setDescription("msc.deleted", msc.getSimpleAddress());
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
