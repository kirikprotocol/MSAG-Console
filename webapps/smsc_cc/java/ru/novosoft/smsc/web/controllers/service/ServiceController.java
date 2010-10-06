package ru.novosoft.smsc.web.controllers.service;

import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscConfigurationController;


/**
 * @author Artem Snopkov
 */
public class ServiceController extends SmscConfigurationController {

  protected SmeManager mngr;

  protected ServiceController() {
    super(WebContext.getInstance().getSmeManager());
    mngr = WebContext.getInstance().getSmeManager();
  }

}
