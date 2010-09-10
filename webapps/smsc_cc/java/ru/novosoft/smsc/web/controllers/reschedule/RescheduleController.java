package ru.novosoft.smsc.web.controllers.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.controllers.SettingsController;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleController extends SettingsController<RescheduleSettings> {
  protected RescheduleController() {
    super(ConfigType.Reschedule);
  }

  @Override
  protected RescheduleSettings loadSettings() throws AdminException {
    return getConfiguration().getRescheduleSettings();
  }

  @Override
  protected void saveSettings(RescheduleSettings settings) throws AdminException {
    getConfiguration().updateRescheduleSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected RescheduleSettings cloneSettings(RescheduleSettings settings) {
    return settings.cloneSettings();
  }
}
