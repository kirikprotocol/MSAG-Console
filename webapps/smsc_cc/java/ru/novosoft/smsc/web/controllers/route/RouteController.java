package ru.novosoft.smsc.web.controllers.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.config.SettingsManager;
import ru.novosoft.smsc.web.controllers.SettingsMController;

/**
 * @author Artem Snopkov
 */
public class RouteController extends SettingsMController<RouteSubjectSettings> {

  protected RouteSubjectManager mngr;

  private boolean initFailed;

  protected RouteController() {
    super(WebContext.getInstance().getRouteSubjectManager());
    mngr = WebContext.getInstance().getRouteSubjectManager();

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
