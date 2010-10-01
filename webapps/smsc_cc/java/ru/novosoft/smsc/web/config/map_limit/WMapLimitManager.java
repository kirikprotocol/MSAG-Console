package ru.novosoft.smsc.web.config.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.map_limit.CongestionLevel;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WMapLimitManager extends BaseSettingsManager<MapLimitSettings> implements MapLimitManager {

  private final MapLimitManager wrapped;
  private final Journal j;

  public WMapLimitManager(MapLimitManager wrapped, Journal journal, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = journal;
  }

  @Override
  protected void _updateSettings(MapLimitSettings newSettings) throws AdminException {
    MapLimitSettings oldSettings = getSettings();

    wrapped.updateSettings(newSettings);

    {
      List<Method> getters = DiffHelper.getGetters(MapLimitSettings.class, "getCongestionLevels");
      List<Object> oldValues = DiffHelper.callGetters(getters, oldSettings);
      List<Object> newValues = DiffHelper.callGetters(getters, newSettings);
      DiffHelper.logChanges(j, JournalRecord.Subject.MAP_LIMIT, oldValues, newValues, getters, user);
    }
    CongestionLevel[] oldLevels = oldSettings.getCongestionLevels();
    CongestionLevel[] newLevels = newSettings.getCongestionLevels();
    List<Method> getters = DiffHelper.getGetters(CongestionLevel.class);
    for (int i = 0; i < MapLimitSettings.MAX_CONGESTION_LEVELS; i++) {
      CongestionLevel oldLevel = oldLevels[i];
      CongestionLevel newLevel = newLevels[i];
      List<Object> oldValues = DiffHelper.callGetters(getters, oldLevel);
      List<Object> newValues = DiffHelper.callGetters(getters, newLevel);
      DiffHelper.logChanges(j, JournalRecord.Subject.MAP_LIMIT, oldValues, newValues, getters, user, "map_limis_cong_level_changed", Integer.toString(i));
    }
  }

  public MapLimitSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public MapLimitSettings cloneSettings(MapLimitSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
