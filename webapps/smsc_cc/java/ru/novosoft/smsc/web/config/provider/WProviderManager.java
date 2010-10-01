package ru.novosoft.smsc.web.config.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.ProviderSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

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
    ProviderSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    
    for (Provider provider : settings.getProviders()) {
      Provider oc = oldSettings.getProvider(provider.getId());
      if (oc == null) {
        JournalRecord r = j.addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.PROVIDER, user);
        r.setDescription("provider.added", provider.getName(), provider.getId() + "");
      } else if (!provider.getName().equals(oc.getName())) {
        JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.PROVIDER, user);
        r.setDescription("provider.renamed", provider.getId() + "", oc.getName(), provider.getName());
      }
    }
  }

  public ProviderSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public ProviderSettings cloneSettings(ProviderSettings settings) {
    return settings.cloneSettings();
  }
}
