package ru.novosoft.smsc.web.config.resource;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.resource.ResourceSettings;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;
import java.util.Map;

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

  public void addResourceSettings(String locale, ResourceSettings resources) throws AdminException {
    boolean contains = containsLocale(locale);
    wrapped.addResourceSettings(locale, resources);
    if (contains)
      j.user(user).change("change").resource(locale);  // todo Надо детализировать изменения.
    else
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
