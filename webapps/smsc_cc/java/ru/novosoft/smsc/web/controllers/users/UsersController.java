package ru.novosoft.smsc.web.controllers.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.controllers.SettingsController;

/**
 * author: alkhal
 */
public class UsersController extends SettingsController<UsersSettings> {
  protected UsersController() {
    super(ConfigType.User);
  }

  @Override
  protected UsersSettings loadSettings() throws AdminException {
    return getConfiguration().getUsersSettings();
  }

  @Override
  protected void saveSettings(UsersSettings settings) throws AdminException {
    getConfiguration().updateUsersSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected UsersSettings cloneSettings(UsersSettings settings) {
    return settings.cloneSettings();
  }
}
