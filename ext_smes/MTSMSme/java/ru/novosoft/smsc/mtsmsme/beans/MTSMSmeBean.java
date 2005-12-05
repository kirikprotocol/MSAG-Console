package ru.novosoft.smsc.mtsmsme.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.mtsmsme.backend.MTSMSmeContext;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 16:30:41
 * To change this template use Options | File Templates.
 */
public class MTSMSmeBean extends PageBean
{
  private String mbMenu = null;
  protected String smeId  = "MTSMSme";

  private MTSMSmeContext mtsmSmeContext = null;

  public byte getSmeStatus()
  {
    try {
      return appContext.getHostsManager().getServiceInfo(smeId).getStatus();
    } catch (Exception e) {
      logger.error("Couldn't obtain MTSMSme status", e);
      return  ServiceInfo.STATUS_UNKNOWN;
    }
  }
  public boolean isSmeRunning() {
      try {
        return appContext.getHostsManager().getServiceInfo(smeId).isOnline();
      } catch (Exception e) {
        logger.error("Couldn't obtain MTSMSme status", e);
        return false;
      }
  }

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)  return result;

    try {
      mtsmSmeContext = MTSMSmeContext.getInstance(appContext, smeId);
    } catch (Throwable e) {
      logger.error("Couldn't init MTSMSme", e);
      return error("mtsmsme.error.init", e);
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    try {
      smeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Could not discover sme id", e);
      error("mtsmsme.error.discover_sme_id", smeId, e);
    }
    return super.process(request);
  }

  public String getMbMenu() {
    return mbMenu;
  }
  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }

  protected Config getConfig() {
    return mtsmSmeContext.getConfig();
  }
  public MTSMSmeContext getMTSMSmeContext() {
    return mtsmSmeContext;
  }

  public String getSmeId() {
    return smeId;
  }

}
