package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;

import java.lang.reflect.Method;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class SmscSettingsDiffHelper extends DiffHelper {

  SmscSettingsDiffHelper(JournalRecord.Subject subject) {
    super(subject);
  }

  public void logChanges(Journal j, SmscSettings oldSettings, SmscSettings newSettings, String user) {

    {
      CommonSettings oldCommon = oldSettings.getCommonSettings();
      CommonSettings newCommon = newSettings.getCommonSettings();
      List<Method> getters = getGetters(CommonSettings.class);
      List<Object> oldValues = callGetters(getters, oldCommon);
      List<Object> newValues = callGetters(getters, newCommon);
      logChanges(j, oldValues, newValues, getters, user);
    }

    {
      List<Method> getters = getGetters(InstanceSettings.class);
      for (int i = 0; i < oldSettings.getSmscInstancesCount(); i++) {
        InstanceSettings old = oldSettings.getInstanceSettings(i);
        InstanceSettings nw = newSettings.getInstanceSettings(i);
        List<Object> oldValues = callGetters(getters, old);
        List<Object> newValues = callGetters(getters, nw);
        logChanges(j, oldValues, newValues, getters, user);
      }
    }
  }

}
