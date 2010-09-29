package ru.novosoft.smsc.web.config.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WAliasManager implements AliasManager {

  private final AliasManager wrapped;
  private final String user;
  private final Journal j;

  public WAliasManager(AliasManager wrapped, String user, Journal j) {
    this.wrapped = wrapped;
    this.user = user;
    this.j = j;
  }

  public void addAlias(Alias alias) throws AdminException {
    wrapped.addAlias(alias);
    JournalRecord r = j.addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.ALIAS, user);
    r.setDescription("alias.added", alias.getAddress().getNormalizedAddress(), alias.getAlias().getNormalizedAddress(), String.valueOf(alias.isHide()));
  }

  public void deleteAlias(Address alias) throws AdminException {
    wrapped.deleteAlias(alias);
    JournalRecord r = j.addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.ALIAS, user);
    r.setDescription("alias.deleted", alias.getNormalizedAddress());
  }

  public AliasSet getAliases() throws AdminException {
    return wrapped.getAliases();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
