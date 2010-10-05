package ru.novosoft.smsc.web.config.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;
import java.util.Map;
import static ru.novosoft.smsc.web.config.DiffHelper.*;

/**
 * @author Artem Snopkov
 */
public class WFraudManager extends BaseSettingsManager<FraudSettings> implements FraudManager {
  
  private final FraudManager wrapped;
  private final Journal j;
  
  public WFraudManager(FraudManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(FraudSettings newSettings) throws AdminException {
    FraudSettings oldSettings = getSettings();
    wrapped.updateSettings(newSettings);    

    findChanges(oldSettings, newSettings, FraudSettings.class, new ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("property_changed", "enableCheck", valueToString(oldValue), valueToString(newValue)).fraud();
      }
    }, "whiteList");

    Collection<Address> oldAdresses = oldSettings.getWhiteList();
    Collection<Address> newAdresses = newSettings.getWhiteList();
    for (Address oldAddr : oldAdresses) {
      if(!newAdresses.contains(oldAddr))
        j.user(user).change("addr_removed", oldAddr.toString()).fraud();
    }
    for (Address newAddr : newAdresses) {
      if(!oldAdresses.contains(newAddr))
        j.user(user).change("addr_removed", newAddr.toString()).fraud();
    }
  }

  public FraudSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public FraudSettings cloneSettings(FraudSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
