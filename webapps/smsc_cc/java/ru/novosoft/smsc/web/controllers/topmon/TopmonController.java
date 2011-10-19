package ru.novosoft.smsc.web.controllers.topmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

/**
 * author: Aleksandr Khalitov
 */
public class TopmonController extends SmscController{
   private String locale;

  private Integer instance;

  private Integer port;

  private boolean show;

  public TopmonController() {
    locale = getLocale().getLanguage();
    String s = getRequestParameter("instance");
    if(s != null && s.length()>0) {
      instance = Integer.parseInt(s);
      try {
        port = WebContext.getInstance().getTopMonitorManager().getAppletPort(instance);
        show = true;
      } catch (AdminException e) {
        addError(e);
      }
    }
  }

  public Integer getPort() {
    return port;
  }

  public Integer getInstance() {
    return instance;
  }

  public boolean isShow() {
    return show;
  }

  public String getLocaleS() {
    return locale;
  }
}
