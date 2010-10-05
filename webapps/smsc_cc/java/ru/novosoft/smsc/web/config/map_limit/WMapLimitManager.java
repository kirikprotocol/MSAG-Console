package ru.novosoft.smsc.web.config.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.map_limit.CongestionLevel;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Map;

import static ru.novosoft.smsc.web.config.DiffHelper.*;

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

    findChanges(oldSettings, newSettings, MapLimitSettings.class, new ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("property_changed", propertyName, valueToString(oldValue), valueToString(newValue));
      }
    }, "congestionLevels");

    CongestionLevel[] oldLevels = oldSettings.getCongestionLevels();
    CongestionLevel[] newLevels = newSettings.getCongestionLevels();
    for (int i = 0; i < MapLimitSettings.MAX_CONGESTION_LEVELS; i++) {
      CongestionLevel oldLevel = oldLevels[i];
      CongestionLevel newLevel = newLevels[i];
      final int congestionLenvelNumber = i;

      findChanges(oldLevel, newLevel, CongestionLevel.class, new ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("cong_level_changed", propertyName, valueToString(oldValue), valueToString(newValue), valueToString(congestionLenvelNumber)).mapLimit();
        }
      });
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
