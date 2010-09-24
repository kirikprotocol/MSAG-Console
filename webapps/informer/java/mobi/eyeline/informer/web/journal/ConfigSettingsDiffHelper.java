package mobi.eyeline.informer.web.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.informer.InformerSettings;

import java.lang.reflect.Method;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class ConfigSettingsDiffHelper extends DiffHelper{      // todo убрать public

  public ConfigSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, InformerSettings oldSettings, InformerSettings newSettings, String user) throws AdminException {
    List<Method> getters = getGetters(InformerSettings.class);
    List<Object> oldValues = callGetters(getters, oldSettings);
    List<Object> newValues = callGetters(getters, newSettings);
    logChanges(j, oldValues, newValues, getters, user);
  }
}
