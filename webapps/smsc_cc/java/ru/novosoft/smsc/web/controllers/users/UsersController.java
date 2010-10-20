package ru.novosoft.smsc.web.controllers.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

/**
 * author: alkhal
 */
public class UsersController extends SettingsMController<UsersSettings> {

  private boolean initFailed;

  public UsersController() {
    super(WebContext.getInstance().getUserManager());

    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
    }
  }

  public boolean isInitFailed() {
    return initFailed;
  }
}
