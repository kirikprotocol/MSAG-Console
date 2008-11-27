package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:30:08 PM
 */
public class InfoSmeBean extends PageBean
{
  public static final int RESULT_APPLY      = PageBean.PRIVATE_RESULT + 0;
  public static final int RESULT_STAT       = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_STATUSES   = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_OPTIONS    = PageBean.PRIVATE_RESULT + 3;
  public static final int RESULT_DRIVERS    = PageBean.PRIVATE_RESULT + 4;
  public static final int RESULT_PROVIDERS  = PageBean.PRIVATE_RESULT + 5;
  public static final int RESULT_TASKS      = PageBean.PRIVATE_RESULT + 6;
  public static final int RESULT_SHEDULES   = PageBean.PRIVATE_RESULT + 7;
  public static final int RESULT_MESSAGES   = PageBean.PRIVATE_RESULT + 8;
  public static final int RESULT_DELIVERIES = PageBean.PRIVATE_RESULT + 9;
  public static final int RESULT_DLSTAT     = PageBean.PRIVATE_RESULT + 10;
  public static final int RESULT_BLACK_LIST = PageBean.PRIVATE_RESULT + 11;
  public static final int RESULT_RETRY_POLICIES = PageBean.PRIVATE_RESULT + 12;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 13;

  private String mbMenu = null;

  public static final String INFOSME_ADMIN_ROLE  = "infosme-admin";
  public static final String INFOSME_MARKET_ROLE = "infosme-market";

  private String smeId = "InfoSme";
  private Config config = null;
  private InfoSmeContext infoSmeContext = null;
  private InfoSme infoSme = null;
  private boolean smeRunning = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      infoSmeContext = InfoSmeContext.getInstance(appContext, smeId);
      infoSme = infoSmeContext.getInfoSme();
      smeRunning = infoSme.getInfo().isOnline();
      config = infoSmeContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get InfoSME config", e);
      return error("infosme.error.config_load", e);
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    /* try {
      smeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Could not discover sme id", e);
      error("Could not discover sme id, \"" + smeId + "\" assumed", e);
    }*/

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if ("apply".equals(mbMenu))           result = RESULT_APPLY;
    else if ("stat".equals(mbMenu))       result = RESULT_STAT;
    else if ("statuses".equals(mbMenu))   result = RESULT_STATUSES;
    else if ("options".equals(mbMenu))    result = RESULT_OPTIONS;
    else if ("drivers".equals(mbMenu))    result = RESULT_DRIVERS;
    else if ("providers".equals(mbMenu))  result = RESULT_PROVIDERS;
    else if ("tasks".equals(mbMenu))      result = RESULT_TASKS;
    else if ("shedules".equals(mbMenu))   result = RESULT_SHEDULES;
    else if ("messages".equals(mbMenu))   result = RESULT_MESSAGES;
    else if ("deliveries".equals(mbMenu)) result = RESULT_DELIVERIES;
    else if ("black_list".equals(mbMenu)) result = RESULT_BLACK_LIST;
    else if ("retryPolicies".equals(mbMenu)) result = RESULT_RETRY_POLICIES;

      mbMenu = null;
      return result;
  }

  public String getMbMenu()
  {
    return mbMenu;
  }

  public void setMbMenu(String mbMenu)
  {
    this.mbMenu = mbMenu;
  }

  protected Config getConfig()
  {
    return config;
  }

  public InfoSmeContext getInfoSmeContext()
  {
    return infoSmeContext;
  }

  public InfoSme getInfoSme()
  {
    return infoSme;
  }

  public boolean isBlackListEnabled() {
    return infoSmeContext.getBlackListManager().isEnabled();
  }

  public boolean isSmeRunning()
  {
    return smeRunning;
  }

  public String getSmeId()
  {
    return smeId;
  }
}
