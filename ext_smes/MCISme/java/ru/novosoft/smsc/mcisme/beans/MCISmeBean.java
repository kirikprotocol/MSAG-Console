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
  public final static int RESULT_APPLY       = PageBean.PRIVATE_RESULT + 0;
  public final static int RESULT_STAT        = PageBean.PRIVATE_RESULT + 1;
  public final static int RESULT_STATUSES    = PageBean.PRIVATE_RESULT + 2;
  public final static int RESULT_OPTIONS     = PageBean.PRIVATE_RESULT + 3;
  public final static int RESULT_CIRCUITS    = PageBean.PRIVATE_RESULT + 4;
  public final static int RESULT_DRIVERS     = PageBean.PRIVATE_RESULT + 5;
  public final static int RESULT_TEMPLATES   = PageBean.PRIVATE_RESULT + 6;
  public final static int RESULT_INFORM_ADD  = PageBean.PRIVATE_RESULT + 7;
  public final static int RESULT_INFORM_EDIT = PageBean.PRIVATE_RESULT + 8;
  public final static int RESULT_NOTIFY_ADD  = PageBean.PRIVATE_RESULT + 9;
  public final static int RESULT_NOTIFY_EDIT = PageBean.PRIVATE_RESULT + 10;
  protected final static int PRIVATE_RESULT  = PageBean.PRIVATE_RESULT + 11;

  public final static String TEMPLATES_SECTION_NAME        = "MCISme.Templates";
  public final static String INFORM_TEMPLATES_SECTION_NAME = TEMPLATES_SECTION_NAME+".Inform";
  public final static String NOTIFY_TEMPLATES_SECTION_NAME = TEMPLATES_SECTION_NAME+".Notify";

  public final static String MCI_PROF_DS_FILE     = "executor.pool.properties";
  public final static String MCI_PROF_MTF_FILE    = "templates.properties";
  public final static String MCI_PROF_MSC_FILE    = "commutator.properties";
  public final static String MCI_PROF_DS_COMMENT  = "MCI Profile DataSource settings";
  public final static String MCI_PROF_MTF_COMMENT = "MCI Profile messages templates settings";
  public final static String MCI_PROF_MSC_COMMENT = "MCI Profile MSC settings";

  public final static String MCI_PROF_LOCATION_PARAM = "MCISme.MCIProfLocation";

  public final static int RELEASE_PREFIXED_STRATEGY = 0x01;
  public final static int RELEASE_REDIRECT_STRATEGY = 0x02; // MTS default strategy

  private String mbMenu = null;

  private String smeId         = "MCISme";
  private String profilerSmeId = "MCIProf";

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

    if      ("apply".equals(mbMenu))     return RESULT_APPLY;
    else if ("stat".equals(mbMenu))      return RESULT_STAT;
    else if ("statuses".equals(mbMenu))  return RESULT_STATUSES;
    else if ("options".equals(mbMenu))   return RESULT_OPTIONS;
    else if ("circuits".equals(mbMenu))  return RESULT_CIRCUITS;
    else if ("templates".equals(mbMenu)) return RESULT_TEMPLATES;
    else if ("drivers".equals(mbMenu))   return RESULT_DRIVERS;
    else return result;
  }

  public String getMbMenu() {
    return mbMenu;
  }
  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }

  protected void restoreConfig() {
    try {
      mciSmeContext.resetConfig();
      config = mciSmeContext.getConfig();
    } catch (Throwable th) {
      logger.error("Restore config failed", th);
    }
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
  public String getProfilerSmeId() {
    return profilerSmeId;
  }
}
