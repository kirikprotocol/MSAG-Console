package ru.novosoft.smsc.web.config.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.region.RegionManager;
import ru.novosoft.smsc.admin.region.RegionSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;

import static ru.novosoft.smsc.web.config.DiffHelper.*;
import ru.novosoft.smsc.web.journal.Journal;

/**
 * @author Artem Snopkov
 */
public class WRegionManager extends BaseSettingsManager<RegionSettings> implements RegionManager {

  private final RegionManager wrapped;
  private final Journal j;

  public WRegionManager(RegionManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(RegionSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  public RegionSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public RegionSettings cloneSettings(RegionSettings settings) {
    return settings.cloneSettings();
  }

  protected void logChanges(RegionSettings oldSettings, RegionSettings newSettings) {
    findChanges(oldSettings, newSettings, RegionSettings.class, new ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("settings_changed", propertyName, valueToString(oldValue), valueToString(newValue));
      }
    }, "regions");

    for (Region r : newSettings.getRegions()) {
      final Region oldRegion = oldSettings.getRegion(r.getId());
      if (oldRegion == null)
        j.user(user).add().region(r.getId() + "", r.getName());
      else
        findChanges(oldRegion, r, Region.class, new ChangeListener() {
          public void foundChange(String propertyName, Object oldValue, Object newValue) {
            j.user(user).change("property_changed", propertyName, valueToString(oldValue), valueToString(newValue)).region(oldRegion.getId() + "");
          }
        });
    }

    for (Region r : oldSettings.getRegions()) {
      if (oldSettings.getRegion(r.getId()) == null)
        j.user(user).remove().region(r.getId() + "", r.getName());
    }
  }
}
