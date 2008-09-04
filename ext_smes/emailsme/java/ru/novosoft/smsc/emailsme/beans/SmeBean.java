package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.emailsme.backend.SmeContext;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;

import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 15:41:47
 * To change this template use Options | File Templates.
 */
public class SmeBean extends PageBean
{
  public static final int RESULT_INDEX = PageBean.PRIVATE_RESULT + 0;
  public static final int RESULT_OPTIONS = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_PROFILES = PageBean.PRIVATE_RESULT + 3;
  public static final int RESULT_STATS = PageBean.PRIVATE_RESULT + 4;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 5;

  private SmeContext smeContext = null;
  private Config config = null;
  private String mbMenu = null;
  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      smeContext = SmeContext.getInstance(appContext);
      config = smeContext.getConfig();
    } catch (Throwable e) {
      logger.debug("Couldn't get SME config", e);
      return error("Could not get SME config", e);
    }

    if ("apply".equals(mbMenu))
      return RESULT_INDEX;
    else if ("options".equals(mbMenu))
      return RESULT_OPTIONS;
    else if ("profiles".equals(mbMenu))
      return RESULT_PROFILES;
    else if ("stats".equals(mbMenu))
      return RESULT_STATS;
    else
      return result;
  }

  public SmeContext getSmeContext()
  {
    return smeContext;
  }

  public Config getConfig()
  {
    return config;
  }

  public String getMbMenu()
  {
    return mbMenu;
  }

  public void setMbMenu(String mbMenu)
  {
    this.mbMenu = mbMenu;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public boolean isServiceOnline()
  {
    try {
      System.out.println("isServiceOnline");
      return appContext.getHostsManager().getServiceInfo(SmeContext.SME_ID).isOnline();
    } catch (AdminException e) {
      logger.error("Couldn't get EMailSme status", e);
      return false;
    }
  }
}
