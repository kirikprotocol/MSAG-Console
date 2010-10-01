package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface ProviderManager {
  
  ProviderSettings getSettings() throws AdminException;

  void updateSettings(ProviderSettings settings) throws AdminException;
}
