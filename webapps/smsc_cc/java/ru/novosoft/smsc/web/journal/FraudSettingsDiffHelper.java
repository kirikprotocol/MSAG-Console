package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Max Kollegov
 */
class FraudSettingsDiffHelper extends DiffHelper {


  public FraudSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, FraudSettings oldSettings, FraudSettings newSettings, String user) {
    if(oldSettings.isEnableCheck()!= newSettings.isEnableCheck()) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("fraudsettings_changed", "enableCheck", Boolean.valueOf(oldSettings.isEnableCheck()).toString(), Boolean.valueOf(newSettings.isEnableCheck()).toString());
    }
    if(oldSettings.isEnableReject()!= newSettings.isEnableReject()) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("fraudsettings_changed", "enableReject", Boolean.valueOf(oldSettings.isEnableReject()).toString(), Boolean.valueOf(newSettings.isEnableReject()).toString());
    }
    if(oldSettings.getTail()!= newSettings.getTail()) {
      j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("fraudsettings_changed", "tail", Integer.valueOf(oldSettings.getTail()).toString(), Integer.valueOf(newSettings.getTail()).toString());
    }

    Collection<Address> oldAdresses = oldSettings.getWhiteList();
    Collection<Address> newAdresses = newSettings.getWhiteList();
    for (Address oldAddr : oldAdresses) {
      if(!newAdresses.contains(oldAddr)) {
         j.addRecord(JournalRecord.Type.REMOVE, subject, user).setDescription("fraudsettings_removed", oldAddr.toString());
      }
    }
    for (Address newAddr : newAdresses) {
      if(!oldAdresses.contains(newAddr)) {
         j.addRecord(JournalRecord.Type.ADD, subject, user).setDescription("fraudsettings_added", newAddr.toString());
      }
    }

  }
}