package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface MapLimitManager extends SmscConfiguration {

  public MapLimitSettings getSettings() throws AdminException;

  public void updateSettings(MapLimitSettings settings) throws AdminException;
}
