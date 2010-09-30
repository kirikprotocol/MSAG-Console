package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface SettingsManager<S> {

  public S getSettings() throws AdminException;

  public void updateSettings(S settings) throws AdminException;

  public S cloneSettings(S settings);

  public long getLastUpdateTime();

  public String getLastUpdateUser();
}
