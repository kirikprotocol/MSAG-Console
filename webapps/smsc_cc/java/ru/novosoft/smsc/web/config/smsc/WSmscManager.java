package ru.novosoft.smsc.web.config.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;

import static ru.novosoft.smsc.web.config.DiffHelper.*;
import static ru.novosoft.smsc.web.config.DiffHelper.findChanges;

import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WSmscManager extends BaseSettingsManager<SmscSettings> implements SmscManager {

  private final SmscManager wrapped;
  private final Journal j;

  public WSmscManager(SmscManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(SmscSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(SmscSettings oldSettings, SmscSettings settings) {
    findChanges(oldSettings.getCommonSettings(), settings.getCommonSettings(), CommonSettings.class, new ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("common_property_changed", propertyName, valueToString(oldValue), valueToString(newValue)).smsc();
      }
    });

    for (int i = 0; i < settings.getSmscInstancesCount(); i++) {
      final int instNumber = i;
      findChanges(oldSettings.getInstanceSettings(i), settings.getInstanceSettings(i), InstanceSettings.class, new ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("instance_property_changed", propertyName, valueToString(oldValue), valueToString(newValue)).smsc(instNumber + "");
        }
      });
    }
  }

  public SmscSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public void startSmsc(int instanceNumber) throws AdminException {
    wrapped.startSmsc(instanceNumber);
    j.user(user).start().smsc(instanceNumber + "");
  }

  public void stopSmsc(int instanceNumber) throws AdminException {
    wrapped.stopSmsc(instanceNumber);
    j.user(user).stop().smsc(instanceNumber + "");
  }

  public void switchSmsc(int instanceNumber, String toHost) throws AdminException {
    wrapped.switchSmsc(instanceNumber, toHost);
    j.user(user).switchTo(toHost).smsc(instanceNumber + "");
  }

  public String getSmscOnlineHost(int instanceNumber) throws AdminException {
    return wrapped.getSmscOnlineHost(instanceNumber);
  }

  public List<String> getSmscHosts(int instanceNumber) throws AdminException {
    return wrapped.getSmscHosts(instanceNumber);
  }

  public SmscSettings cloneSettings(SmscSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
