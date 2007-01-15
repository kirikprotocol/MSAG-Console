package ru.novosoft.smsc.unibalance.bean;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.unibalance.backend.SmeContext;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.Properties;

/**
 * User: artem
 * Date: 12.01.2007
 */
public class SmeBean extends PageBean{

  private SmeContext smeContext = null;
  private Properties config = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      smeContext = SmeContext.getInstance(appContext);
      config = smeContext.getConfig();
    } catch (Throwable e) {
      logger.debug("Couldn't get unibalance SME config", e);
      return error("Could not get unibalance SME config", e);
    }

    return RESULT_OK;
  }

  public SmeContext getSmeContext() {
    return smeContext;
  }

  public Properties getConfig() {
    return config;
  }

  public boolean isServiceOnline()
  {
    try {
      return appContext.getHostsManager().getServiceInfo(SmeContext.SME_ID).isOnline();
    } catch (AdminException e) {
      logger.error("Couldn't get unibalance SME status", e);
      return false;
    }
  }
}
