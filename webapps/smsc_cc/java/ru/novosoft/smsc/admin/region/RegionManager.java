package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface RegionManager {
  RegionSettings getSettings() throws AdminException;

  void updateSettings(RegionSettings s) throws AdminException;
}
