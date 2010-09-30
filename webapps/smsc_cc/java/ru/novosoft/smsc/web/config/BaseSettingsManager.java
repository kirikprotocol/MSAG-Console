package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminException;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public abstract class BaseSettingsManager<S> implements SettingsManager<S> {

  private static Map<String, Revision> revisions = new HashMap<String, Revision>();

  protected final String user;
  private final String id;

  protected BaseSettingsManager(String user) {
    this.user = user;
    this.id = getClass().getCanonicalName();
  }

  protected abstract void _updateSettings(S settings) throws AdminException;

  public final void updateSettings(S settings) throws AdminException {
    _updateSettings(settings);
    revisions.put(id, new Revision(user, System.currentTimeMillis()));
  }

  public final long getLastUpdateTime() {
    Revision rev = revisions.get(id);
    return rev == null ? 0 : rev.time;
  }

  public final String getLastUpdateUser() {
    Revision rev = revisions.get(id);
    return rev == null ? null : rev.user;
  }

  private static class Revision {
    private String user;
    private long time;

    private Revision(String user, long time) {
      this.user = user;
      this.time = time;
    }
  }
}
