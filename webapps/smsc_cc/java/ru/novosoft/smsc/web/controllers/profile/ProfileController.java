package ru.novosoft.smsc.web.controllers.profile;

import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscConfigurationController;

/**
 * @author Artem Snopkov
 */
public class ProfileController extends SmscConfigurationController {

  protected ProfileManager mngr;

  public ProfileController() {
    super(WebContext.getInstance().getProfileManager());

    mngr = WebContext.getInstance().getProfileManager();
  }
}
