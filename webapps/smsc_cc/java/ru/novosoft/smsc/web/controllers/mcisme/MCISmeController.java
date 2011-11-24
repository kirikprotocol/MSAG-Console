package ru.novosoft.smsc.web.controllers.mcisme;

import ru.novosoft.smsc.web.WebContext;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeController {

  public boolean isMCISMEDeployed() {
    return WebContext.getInstance().getMciSmeManager() != null;
  }
}
