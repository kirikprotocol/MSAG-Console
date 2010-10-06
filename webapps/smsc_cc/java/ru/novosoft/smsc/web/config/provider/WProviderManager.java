package ru.novosoft.smsc.web.config.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.ProviderSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

/**
 * @author Artem Snopkov
 */
public class WProviderManager extends BaseSettingsManager<ProviderSettings> implements ProviderManager {
  
  private final ProviderManager wrapped;
  private final Journal j;
  
  public WProviderManager(ProviderManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(ProviderSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(ProviderSettings oldSettings, ProviderSettings newSettings) {
    for (Provider provider : newSettings.getProviders()) {
      Provider oc = oldSettings.getProvider(provider.getId());
      if (oc == null)
        j.user(user).add().provider(provider.getName(), provider.getId() + "");
      else if (!provider.getName().equals(oc.getName()))
        j.user(user).change("rename", oc.getName(), provider.getName()).provider(provider.getId() + "");
    }
  }

  public ProviderSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public ProviderSettings cloneSettings(ProviderSettings settings) {
    return settings.cloneSettings();
  }
}
