package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.emailsme.backend.SmeContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import java.security.Principal;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 15:41:03
 * To change this template use Options | File Templates.
 */
public class Index extends SmeBean
{
  private String mbApplyAll = null;
  private String mbResetAll = null;
  private String mbStart = null;
  private String mbStop = null;

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbApplyAll != null)
      return applyAll();
    if (mbResetAll != null)
      return resetAll();
    if (mbStart != null)
      return start();
    if (mbStop != null)
      return stop();

    return result;
  }

  private int stop()
  {
    try {
      appContext.getHostsManager().shutdownService(SmeContext.SME_ID);
    } catch (AdminException e) {
      return error("Could not stop Email SME", e);
    }
    return RESULT_DONE;
  }

  private int start()
  {
    try {
      appContext.getHostsManager().startService(SmeContext.SME_ID);
    } catch (AdminException e) {
      return error("Could not start Email SME", e);
    }
    return RESULT_DONE;
  }

  private int resetAll()
  {
    try {
      getSmeContext().resetConfig();
    } catch (Throwable e) {
      logger.error("Could not reload config", e);
      return error("Could not reload config", e);
    }
    return RESULT_DONE;
  }

  private int applyAll()
  {
    try {
      getConfig().save();
    } catch (Throwable e) {
      logger.error("Could not save config", e);
      return error("Could not save config", e);
    }
    return RESULT_DONE;
  }

  public String getMbApplyAll()
  {
    return mbApplyAll;
  }

  public void setMbApplyAll(String mbApplyAll)
  {
    this.mbApplyAll = mbApplyAll;
  }

  public String getMbResetAll()
  {
    return mbResetAll;
  }

  public void setMbResetAll(String mbResetAll)
  {
    this.mbResetAll = mbResetAll;
  }

  public String getMbStart()
  {
    return mbStart;
  }

  public void setMbStart(String mbStart)
  {
    this.mbStart = mbStart;
  }

  public String getMbStop()
  {
    return mbStop;
  }

  public void setMbStop(String mbStop)
  {
    this.mbStop = mbStop;
  }
}
