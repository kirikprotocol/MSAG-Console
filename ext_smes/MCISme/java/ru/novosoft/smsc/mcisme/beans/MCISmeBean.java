package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.mcisme.backend.MCISmeContext;
import ru.novosoft.smsc.mcisme.backend.MCISme;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 16:30:41
 * To change this template use Options | File Templates.
 */
public class MCISmeBean extends PageBean
{
  public static final int RESULT_APPLY = PageBean.PRIVATE_RESULT + 0;
  public static final int RESULT_STAT = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_STATUSES = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_OPTIONS = PageBean.PRIVATE_RESULT + 3;
  public static final int RESULT_DRIVERS = PageBean.PRIVATE_RESULT + 4;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 5;

  private String mbMenu = null;

  private String smeId = "MCISme";
  private Config config = null;
  private MCISmeContext mciSmeContext = null;
  private MCISme mciSme = null;
  private boolean smeRunning = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)  return result;

    try {
      mciSmeContext = MCISmeContext.getInstance(appContext, smeId);
      mciSme = mciSmeContext.getMCISme();
      smeRunning = mciSme.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING;
      config = mciSmeContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get MCISme config", e);
      return error("Could not get MCISsme config", e);
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
    if (result != RESULT_OK)  return result;

    if      ("apply".equals(mbMenu))    return RESULT_APPLY;
    else if ("stat".equals(mbMenu))     return RESULT_STAT;
    else if ("statuses".equals(mbMenu)) return RESULT_STATUSES;
    else if ("options".equals(mbMenu))  return RESULT_OPTIONS;
    else if ("drivers".equals(mbMenu))  return RESULT_DRIVERS;
    else return result;
  }

  public String getMbMenu() {
    return mbMenu;
  }
  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }

  protected Config getConfig() {
    return config;
  }
  public MCISmeContext getMCISmeContext() {
    return mciSmeContext;
  }

  public MCISme getMCISme() {
    return mciSme;
  }

  public boolean isSmeRunning() {
    return smeRunning;
  }
  public String getSmeId() {
    return smeId;
  }

}
