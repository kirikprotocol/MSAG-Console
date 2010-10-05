package ru.novosoft.smsc.web.config.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatuses;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WSmeManager implements SmeManager {

  private final SmeManager wrapped;
  private final Journal j;
  private final String user;

  public WSmeManager(SmeManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public void addSme(String smeId, Sme newSme) throws AdminException {
    wrapped.addSme(smeId, newSme);
  }

  public Sme getSme(String smeId) throws AdminException {
    return wrapped.getSme(smeId);
  }

  public boolean removeSme(String smeId) throws AdminException {
    return wrapped.removeSme(smeId);
  }

  public boolean contains(String smeId) {
    return wrapped.contains(smeId);
  }

  public Map<String, Sme> smes() throws AdminException {
    return wrapped.smes();
  }

  public Map<String, SmeSmscStatuses> getSmesSmscStatuses() throws AdminException {
    return wrapped.getSmesSmscStatuses();
  }

  public void disconnectSmeFromSmsc(String... smeIds) throws AdminException {
    wrapped.disconnectSmeFromSmsc(smeIds);
  }

  public void disconnectSmeFromSmsc(List<String> smeIds) throws AdminException {
    wrapped.disconnectSmeFromSmsc(smeIds);
  }

  public SmeServiceStatus getSmeServiceStatus(String smeId) throws AdminException {
    return wrapped.getSmeServiceStatus(smeId);
  }

  public void startSme(String smeId) throws AdminException {
    wrapped.startSme(smeId);
  }

  public void stopSme(String smeId) throws AdminException {
    wrapped.stopSme(smeId);
  }

  public void switchSme(String smeId, String toHost) throws AdminException {
    wrapped.switchSme(smeId, toHost);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
