package ru.novosoft.smsc.web.config.user;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersManager;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import static ru.novosoft.smsc.web.config.DiffHelper.*;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WUserManager extends BaseSettingsManager<UsersSettings> implements UsersManager {

  private final UsersManager wrapped;
  private final Journal j;

  public WUserManager(UsersManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(UsersSettings settings) throws AdminException {
    UsersSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    logChanges(oldSettings, settings);
  }

  public UsersSettings getSettings() throws AdminException {
    return getUsersSettings();
  }

  public UsersSettings cloneSettings(UsersSettings settings) {
    return settings.cloneSettings();
  }

  public UsersSettings getUsersSettings() throws AdminException {
    return wrapped.getUsersSettings();
  }

  protected void logChanges(UsersSettings oldSettings, UsersSettings newSettings) {
    Map<String, User> oldUsers = oldSettings.getUsersMap();
    Map<String, User> newUsers = newSettings.getUsersMap();
    for (Map.Entry<String, User> e : oldUsers.entrySet()) {
      final User oldUser = e.getValue();
      User newUser = newUsers.get(e.getKey());
      if (newUser == null) {
        j.user(user).remove().user(oldUser.getLogin());
      } else {
        findChanges(oldUser, newUser, User.class, new ChangeListener() {
          public void foundChange(String propertyName, Object oldValue, Object newValue) {
            j.user(user).change("property_changed", propertyName, valueToString(oldValue), valueToString(newValue)).user(oldUser.getLogin());
          }
        }, "password");
      }
    }
    for (Map.Entry<String, User> e : newUsers.entrySet()) {
      if (!oldUsers.containsKey(e.getKey())) {
        j.user(user).add().user(e.getValue().getLogin());
      }
    }
  }
}
