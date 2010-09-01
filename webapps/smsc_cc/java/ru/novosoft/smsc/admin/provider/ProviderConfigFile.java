package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
class ProviderConfigFile implements ManagedConfigFile<ProviderSettings> {

  public void save(InputStream oldFile, OutputStream newFile, ProviderSettings conf) throws Exception {
    XmlConfig cfg = new XmlConfig(oldFile);

    XmlConfigSection providersSection = cfg.getOrCreateSection("providers");
    providersSection.clear();

    for (Provider p : conf.getProviders())
      providersSection.setString(String.valueOf(p.getId()), p.getName());

    providersSection.setLong("last used provider id", conf.getLastProviderId());

    cfg.save(newFile);
  }

  public ProviderSettings load(InputStream is) throws Exception {
    XmlConfig cfg = new XmlConfig(is);



    if (cfg.containsSection("providers")) {
      Collection<Provider> providers = new ArrayList<Provider>();
      long lastUsedProviderId = -1;
      XmlConfigSection providersSection = cfg.getSection("providers");
      for (XmlConfigParam p : providersSection.params()) {
        if (p.getName().equals("last used provider id")) {
          lastUsedProviderId = p.getLong();
        } else {
          providers.add(new Provider(Long.parseLong(p.getName()), p.getString()));
        }
      }
      return new ProviderSettings(providers, lastUsedProviderId);
    }

    return new ProviderSettings(new ArrayList<Provider>(), -1);
  }
}
