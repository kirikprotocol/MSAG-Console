package ru.novosoft.smsc.web.config.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatuses;
import static ru.novosoft.smsc.web.config.DiffHelper.*;
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

  public void addSme(final String smeId, Sme newSme) throws AdminException {
    Sme oldSme = getSme(smeId);
    wrapped.addSme(smeId, newSme);
    if (oldSme != null) {
      findChanges(oldSme, newSme, Sme.class, new ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("property_change", propertyName, valueToString(oldValue), valueToString(newValue)).sme(smeId);
        }
      });
    } else
      j.user(user).add().sme(smeId);
  }

  public Sme getSme(String smeId) throws AdminException {
    return wrapped.getSme(smeId);
  }

  public boolean removeSme(String smeId) throws AdminException {
    boolean res = wrapped.removeSme(smeId);
    if (res)
      j.user(user).remove().sme(smeId);
    return res;
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
    for (String smeId : smeIds)
      j.user(user).disconnect().sme(smeId);
  }

  public void disconnectSmeFromSmsc(List<String> smeIds) throws AdminException {
    wrapped.disconnectSmeFromSmsc(smeIds);
    for (String smeId : smeIds)
      j.user(user).disconnect().sme(smeId);
  }

  public SmeServiceStatus getSmeServiceStatus(String smeId) throws AdminException {
    return wrapped.getSmeServiceStatus(smeId);
  }

  public void startSme(String smeId) throws AdminException {
    wrapped.startSme(smeId);
    j.user(user).start().sme(smeId);
  }

  public void stopSme(String smeId) throws AdminException {
    wrapped.stopSme(smeId);
    j.user(user).stop().sme(smeId);
  }

  public void switchSme(String smeId, String toHost) throws AdminException {
    wrapped.switchSme(smeId, toHost);
    j.user(user).switchTo(toHost).sme(smeId);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
