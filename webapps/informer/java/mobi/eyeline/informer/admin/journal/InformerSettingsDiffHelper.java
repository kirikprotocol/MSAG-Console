package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.informer.InformerSettings;

import java.lang.reflect.Method;
import java.util.List;

/**
 * Ищет и журналирует изменения в настройках Informer
 * @author Aleksandr Khalitov
 */
class InformerSettingsDiffHelper extends DiffHelper{

  InformerSettingsDiffHelper(Subject subject) {
    super(subject);
  }

  void logChanges(Journal j, InformerSettings oldSettings, InformerSettings newSettings, String user) throws AdminException {
    List<Method> getters = getGetters(InformerSettings.class);
    List<Object> oldValues = callGetters(getters, oldSettings);
    List<Object> newValues = callGetters(getters, newSettings);
    logChanges(j, oldValues, newValues, getters, user);
  }
}
