package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:30:08 PM
 */
public class InfoSmeBean extends PageBean
{
  public static final int RESULT_APPLY = PageBean.PRIVATE_RESULT + 0;
  public static final int RESULT_STAT = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_STATUSES = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_OPTIONS = PageBean.PRIVATE_RESULT + 3;
  public static final int RESULT_DRIVERS = PageBean.PRIVATE_RESULT + 4;
  public static final int RESULT_PROVIDERS = PageBean.PRIVATE_RESULT + 5;
  public static final int RESULT_TASKS = PageBean.PRIVATE_RESULT + 6;
  public static final int RESULT_SHEDULES = PageBean.PRIVATE_RESULT + 7;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 8;


  private String mbMenu = null;

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
      smeRunning = infoSme.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING;
      config = infoSmeContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get InfoSME config", e);
      return error("Could not get InfoSME config", e);
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    try {
      smeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Could not discover sme id", e);
      error("Could not discover sme id, \"" + smeId + "\" assumed", e);
    }

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if ("apply".equals(mbMenu))
      return RESULT_APPLY;
    else if ("stat".equals(mbMenu))
      return RESULT_STAT;
    else if ("statuses".equals(mbMenu))
      return RESULT_STATUSES;
    else if ("options".equals(mbMenu))
      return RESULT_OPTIONS;
    else if ("drivers".equals(mbMenu))
      return RESULT_DRIVERS;
    else if ("providers".equals(mbMenu))
      return RESULT_PROVIDERS;
    else if ("tasks".equals(mbMenu))
      return RESULT_TASKS;
    else if ("shedules".equals(mbMenu))
      return RESULT_SHEDULES;
    else
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

  public boolean isSmeRunning()
  {
    return smeRunning;
  }

  public String getSmeId()
  {
    return smeId;
  }
}
