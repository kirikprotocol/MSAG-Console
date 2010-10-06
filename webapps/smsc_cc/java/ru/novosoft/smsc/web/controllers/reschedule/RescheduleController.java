package ru.novosoft.smsc.web.controllers.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleController extends SettingsMController<RescheduleSettings> {

  private boolean initFailed;

  protected RescheduleController(boolean checkChanges) {
    super(WebContext.getInstance().getRescheduleManager());

    try {
      init(checkChanges);
    } catch (AdminException e) {
      initFailed = true;
      addError(e);
    }

  }

  public boolean isInitFailed() {
    return initFailed;
  }
}
