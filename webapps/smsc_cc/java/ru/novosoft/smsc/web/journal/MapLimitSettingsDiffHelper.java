package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.map_limit.CongestionLevel;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;

import java.lang.reflect.Method;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class MapLimitSettingsDiffHelper extends DiffHelper {


  public MapLimitSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, MapLimitSettings oldSettings, MapLimitSettings newSettings, String user) {
    {
      List<Method> getters = getGetters(MapLimitSettings.class, "getCongestionLevels");
      List<Object> oldValues = callGetters(getters, oldSettings);
      List<Object> newValues = callGetters(getters, newSettings);
      logChanges(j, oldValues, newValues, getters, user);
    }
    CongestionLevel[] oldLevels = oldSettings.getCongestionLevels();
    CongestionLevel[] newLevels = newSettings.getCongestionLevels();
    List<Method> getters = getGetters(CongestionLevel.class);
    for(int i=0;i<MapLimitSettings.MAX_CONGESTION_LEVELS;i++) {
      CongestionLevel oldLevel = oldLevels[i];
      CongestionLevel newLevel = newLevels[i];
      List<Object> oldValues = callGetters(getters, oldLevel);
      List<Object> newValues = callGetters(getters, newLevel);
      logChanges(j, oldValues, newValues, getters, user, "map_limis_cong_level_changed", Integer.toString(i));
    }
  }
}
