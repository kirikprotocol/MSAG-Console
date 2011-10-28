package ru.novosoft.smsc.web.config.resource;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.resource.ResourceSettings;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;
import java.util.Map;

import static ru.novosoft.smsc.web.config.DiffHelper.findChanges;
import static ru.novosoft.smsc.web.config.DiffHelper.valueToString;

/**
 * @author Artem Snopkov
 */
public class WResourceManager implements ResourceManager {

  private final ResourceManager wrapped;
  private final Journal j;
  private final String user;

  public WResourceManager(ResourceManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public Collection<String> getLocales() throws AdminException {
    return wrapped.getLocales();
  }

  public boolean containsLocale(String locale) throws AdminException {
    return wrapped.containsLocale(locale);
  }

  public void addResourceSettings(final String locale, ResourceSettings resources) throws AdminException {
    ResourceSettings oldSettings = wrapped.getResourceSettings(locale);
    wrapped.addResourceSettings(locale, resources);
    if (oldSettings != null) {
      findChanges(oldSettings, resources, ResourceSettings.class, new DiffHelper.ChangeListener() {
        public void foundChange(String propertyName, Object oldValue, Object newValue) {
          j.user(user).change("property_change", propertyName, valueToString(oldValue), valueToString(newValue)).resource(locale);
        }
      });
    } else
      j.user(user).add().resource(locale);
  }

  public boolean removeResourceSettings(String locale) throws AdminException {
    boolean res = wrapped.removeResourceSettings(locale);
    if (res)
      j.user(user).remove().resource(locale);
    return res;
  }

  public ResourceSettings getResourceSettings(String locale) throws AdminException {
    return wrapped.getResourceSettings(locale);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
