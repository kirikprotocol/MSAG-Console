package ru.novosoft.smsc.web.config.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.Collection;
import java.util.Map;

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
    
    if(oldSettings.isEnableCheck()!= newSettings.isEnableCheck())
      j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.FRAUD, user).setDescription("fraudsettings_changed", "enableCheck", Boolean.valueOf(oldSettings.isEnableCheck()).toString(), Boolean.valueOf(newSettings.isEnableCheck()).toString());
    if(oldSettings.isEnableReject()!= newSettings.isEnableReject())
      j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.FRAUD, user).setDescription("fraudsettings_changed", "enableReject", Boolean.valueOf(oldSettings.isEnableReject()).toString(), Boolean.valueOf(newSettings.isEnableReject()).toString());
    if(oldSettings.getTail()!= newSettings.getTail())
      j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.FRAUD, user).setDescription("fraudsettings_changed", "tail", Integer.valueOf(oldSettings.getTail()).toString(), Integer.valueOf(newSettings.getTail()).toString());

    Collection<Address> oldAdresses = oldSettings.getWhiteList();
    Collection<Address> newAdresses = newSettings.getWhiteList();
    for (Address oldAddr : oldAdresses) {
      if(!newAdresses.contains(oldAddr))
         j.addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.FRAUD, user).setDescription("fraudsettings_removed", oldAddr.toString());
    }
    for (Address newAddr : newAdresses) {
      if(!oldAdresses.contains(newAddr))
         j.addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.FRAUD, user).setDescription("fraudsettings_added", newAddr.toString());
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
