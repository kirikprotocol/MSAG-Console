package ru.novosoft.smsc.web.controllers.perfmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

/**
 * author: Aleksandr Khalitov
 */
public class PerfmonController extends SmscController{

  private String locale = "en";

  private Integer instance;

  private Integer port;

  private boolean show;

  public PerfmonController() {
    locale = getLocale().getLanguage();
    String s = getRequestParameter("instance");
    if(s != null && s.length()>0) {
      instance = Integer.parseInt(s);
      try {
        port = WebContext.getInstance().getPerfMonitorManager().getAppletPort(instance);
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
