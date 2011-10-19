package ru.novosoft.smsc.web.config.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.snmp.*;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Map;

import static ru.novosoft.smsc.web.config.DiffHelper.*;

/**
 * @author Artem Snopkov
 */
public class WSnmpManager extends BaseSettingsManager<SnmpSettings> implements SnmpManager {

  private final SnmpManager wrapped;
  private final Journal j;

  public WSnmpManager(SnmpManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }


  public void getTraps(SnmpFilter filter, SnmpTrapVisitor visitor) throws AdminException {
    wrapped.getTraps(filter, visitor);
  }

  @Override
  protected void _updateSettings(SnmpSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  public SnmpSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public SnmpSettings cloneSettings(SnmpSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }

  protected void logChanges(SnmpSettings oldSettings, SnmpSettings newSettings) {
    oldSettings.getCounterInterval();

    if (oldSettings.getCounterInterval() != newSettings.getCounterInterval()) {
      j.user(user).change("counterInterval.change", Integer.toString(oldSettings.getCounterInterval()),
          Integer.toString(newSettings.getCounterInterval())).snmp();
    }
    logObjectChanges(null, oldSettings.getDefaultSnmpObject(), newSettings.getDefaultSnmpObject());

    Map<String, SnmpObject> oldSnmpObjects = oldSettings.getSnmpObjects();
    Map<String, SnmpObject> newSnmpObjects = newSettings.getSnmpObjects();

    for (String objName : oldSettings.getSnmpObjects().keySet()) {
      if (newSnmpObjects.get(objName) == null) {
        SnmpObject oldSnmpObject = oldSettings.getSnmpObjects().get(objName);
        logCounter(objName, "counter000d", oldSnmpObject.getCounter000d(), null);
        logCounter(objName, "counter0014", oldSnmpObject.getCounter0014(), null);
        logCounter(objName, "counter0058", oldSnmpObject.getCounter0058(), null);
        logCounter(objName, "counterAccepted", oldSnmpObject.getCounterAccepted(), null);
        logCounter(objName, "counterDelivered", oldSnmpObject.getCounterDelivered(), null);
        logCounter(objName, "counterFailed", oldSnmpObject.getCounterFailed(), null);
        logCounter(objName, "counterOther", oldSnmpObject.getCounterOther(), null);
        logCounter(objName, "counterRejected", oldSnmpObject.getCounterRejected(), null);
        logCounter(objName, "counterRetired", oldSnmpObject.getCounterRetried(), null);
        logCounter(objName, "counterSDP", oldSnmpObject.getCounterSDP(), null);
        logCounter(objName, "counterTempError", oldSnmpObject.getCounterTempError(), null);

        logSeverity(objName, "severityRegister", oldSnmpObject.getSeverityRegister(), null);
        logSeverity(objName, "severityRegisterFailed", oldSnmpObject.getSeverityRegisterFailed(), null);
        logSeverity(objName, "severityUnregister", oldSnmpObject.getSeverityUnregister(), null);
        logSeverity(objName, "severityUnregisterFailed", oldSnmpObject.getSeverityUnregisterFailed(), null);
        j.user(user).remove().snmp(objName);
      } else {
        logObjectChanges(objName, oldSnmpObjects.get(objName), newSnmpObjects.get(objName));
      }
    }

    for (String objName : newSettings.getSnmpObjects().keySet()) {
      if (oldSettings.getSnmpObjects().get(objName) == null) {
        SnmpObject newSnmpObject = newSettings.getSnmpObjects().get(objName);
        j.user(user).add().snmp(objName);
        logCounter(objName, "counter000d", null, newSnmpObject.getCounter000d());
        logCounter(objName, "counter0014", null, newSnmpObject.getCounter0014());
        logCounter(objName, "counter0058", null, newSnmpObject.getCounter0058());
        logCounter(objName, "counterAccepted", null, newSnmpObject.getCounterAccepted());
        logCounter(objName, "counterDelivered", null, newSnmpObject.getCounterDelivered());
        logCounter(objName, "counterFailed", null, newSnmpObject.getCounterFailed());
        logCounter(objName, "counterOther", null, newSnmpObject.getCounterOther());
        logCounter(objName, "counterRejected", null, newSnmpObject.getCounterRejected());
        logCounter(objName, "counterRetired", null, newSnmpObject.getCounterRetried());
        logCounter(objName, "counterSDP", null, newSnmpObject.getCounterSDP());
        logCounter(objName, "counterTempError", null, newSnmpObject.getCounterTempError());

        logSeverity(objName, "severityRegister", null, newSnmpObject.getSeverityRegister());
        logSeverity(objName, "severityRegisterFailed", null, newSnmpObject.getSeverityRegisterFailed());
        logSeverity(objName, "severityUnregister", null, newSnmpObject.getSeverityUnregister());
        logSeverity(objName, "severityUnregisterFailed", null, newSnmpObject.getSeverityUnregisterFailed());

      }
    }


  }

  private void logObjectChanges(String objName, SnmpObject oldSnmpObject, SnmpObject newSnmpObject) {
    if (objName == null) objName = "DEFAULT";
    if (oldSnmpObject.isEnabled() != newSnmpObject.isEnabled()) 
      j.user(user).change("object.enableChanged", Boolean.toString(newSnmpObject.isEnabled())).snmp(objName);

    logCounter(objName, "counter000d", oldSnmpObject.getCounter000d(), newSnmpObject.getCounter000d());
    logCounter(objName, "counter0014", oldSnmpObject.getCounter0014(), newSnmpObject.getCounter0014());
    logCounter(objName, "counter0058", oldSnmpObject.getCounter0058(), newSnmpObject.getCounter0058());
    logCounter(objName, "counterAccepted", oldSnmpObject.getCounterAccepted(), newSnmpObject.getCounterAccepted());
    logCounter(objName, "counterDelivered", oldSnmpObject.getCounterDelivered(), newSnmpObject.getCounterDelivered());
    logCounter(objName, "counterFailed", oldSnmpObject.getCounterFailed(), newSnmpObject.getCounterFailed());
    logCounter(objName, "counterOther", oldSnmpObject.getCounterOther(), newSnmpObject.getCounterOther());
    logCounter(objName, "counterRejected", oldSnmpObject.getCounterRejected(), newSnmpObject.getCounterRejected());
    logCounter(objName, "counterRetired", oldSnmpObject.getCounterRetried(), newSnmpObject.getCounterRetried());
    logCounter(objName, "counterSDP", oldSnmpObject.getCounterSDP(), newSnmpObject.getCounterSDP());
    logCounter(objName, "counterTempError", oldSnmpObject.getCounterTempError(), newSnmpObject.getCounterTempError());

    logSeverity(objName, "severityRegister", oldSnmpObject.getSeverityRegister(), newSnmpObject.getSeverityRegister());
    logSeverity(objName, "severityRegisterFailed", oldSnmpObject.getSeverityRegisterFailed(), newSnmpObject.getSeverityRegisterFailed());
    logSeverity(objName, "severityUnregister", oldSnmpObject.getSeverityUnregister(), newSnmpObject.getSeverityUnregister());
    logSeverity(objName, "severityUnregisterFailed", oldSnmpObject.getSeverityUnregisterFailed(), newSnmpObject.getSeverityUnregisterFailed());
  }

  private void logSeverity(String objName, String severityKey, SnmpSeverity severityOld, SnmpSeverity severityNew) {
    if (severityNew == null && severityOld == null) return;
    if (severityOld == null) {
      j.user(user).change("severity.add", severityKey, severityNew.toString()).snmp(objName);
    } else if (severityNew == null) {
      j.user(user).change("severity.remove", severityKey, severityOld.toString()).snmp(objName);
    } else {
      if (!severityOld.equals(severityNew)) {
        j.user(user).change("severity.change", severityKey, severityOld.toString(), severityNew.toString()).snmp(objName);
      }
    }
  }

  private void logCounter(final String objName, final String counterKey, SnmpCounter counterOld, SnmpCounter counterNew) {
    if (counterOld == null && counterNew == null) return;
    if (counterOld == null) {
      j.user(user).change("counter.add", counterKey, Integer.toString(counterNew.getWarning()),
          Integer.toString(counterNew.getMinor()),
          Integer.toString(counterNew.getMajor()),
          Integer.toString(counterNew.getCritical())).snmp(objName);
    } else if (counterNew == null) {
      j.user(user).change("counter.remove", counterKey, Integer.toString(counterOld.getWarning()),
          Integer.toString(counterOld.getMinor()),
          Integer.toString(counterOld.getMajor()),
          Integer.toString(counterOld.getCritical())).snmp(objName);
    } else {
      findChanges(counterOld, counterNew, SnmpCounter.class, new ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("counter.property_changed", counterKey, propertyName, valueToString(oldValue), valueToString(newValue)).snmp(objName);
        }
      });      
    }
  }
}
