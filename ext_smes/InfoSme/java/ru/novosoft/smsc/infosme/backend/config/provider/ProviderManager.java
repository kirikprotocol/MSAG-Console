package ru.novosoft.smsc.infosme.backend.config.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class ProviderManager {

  public static final String PROVIDER_PREFIX = "InfoSme.DataProvider";

  private Map providers = new HashMap();
  private boolean modified;

  public ProviderManager(Config cfg) throws AdminException {
    resetProviders(cfg);
  }

  public synchronized void addProvider(Provider p) {
    providers.put(p.getName(), p);
  }

  public synchronized void removeProvider(String name) {
    providers.remove(name);
    modified = true;
  }

  public synchronized Provider getProvider(String name) {
    return (Provider)providers.get(name);
  }

  public synchronized List getProviders() {
    return new ArrayList(providers.values());
  }

  public synchronized boolean containsProvider(String name) {
    return providers.containsKey(name);
  }

  public synchronized boolean isProvidersChanged() {
    if (modified)
      return true;
    for (Iterator iter = providers.values().iterator(); iter.hasNext();) {
      Provider t = (Provider)iter.next();
      if (t.isModified())
        return true;
    }
    return false;
  }

  public synchronized void applyProviders(Config cfg) throws AdminException {
    try {
      // Remove old providers
      cfg.removeSection(PROVIDER_PREFIX);

      // Add new providers
      for (Iterator iter = providers.values().iterator(); iter.hasNext();) {
        Provider t = (Provider)iter.next();
        t.storeToConfig(cfg);
        t.setModified(false);
      }

      modified = false;

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized void resetProviders(Config cfg) throws AdminException {
    providers.clear();
    try {
      for (Iterator iter = loadProviders(cfg).iterator(); iter.hasNext();) {
        Provider p = (Provider)iter.next();
        providers.put(p.getName(), p);
      }
      modified = false;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  private List loadProviders(Config config) throws AdminException {
    List result = new ArrayList(100);
    for (Iterator i = config.getSectionChildShortSectionNames(PROVIDER_PREFIX).iterator(); i.hasNext();)
      result.add(new Provider((String)i.next(), config));
    return result;
  }
}
