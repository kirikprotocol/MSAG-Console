package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.mcisme.backend.MCISme;
import ru.novosoft.smsc.mcisme.backend.CountersSet;
import ru.novosoft.smsc.mcisme.backend.RuntimeSet;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import javax.servlet.http.HttpServletRequest;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.04.2004
 * Time: 15:03:15
 * To change this template use Options | File Templates.
 */
public abstract class IndexProperties extends MCISmeBean
{
  private MCISme mciSme;
  private boolean mcismeStarted;
  private String mbApply = null;
  private String mbReset = null;
  private String mbStart = null;
  private String mbStop = null;

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)  return result;

    mciSme = getMCISmeContext().getMCISme();

    try {
      getAppContext().getHostsManager().refreshServices();
      mciSme.updateInfo(appContext);
    } catch (AdminException e) {
      logger.error("Could not refresh MCI Sme status", e);
      result = error("Could not refresh MCI Sme status", e);
    }

    mcismeStarted = mciSme.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING;

    if (mbApply != null)  return apply();
    if (mbReset != null)  return reset();
    if (mbStart != null)  return start();
    if (mbStop != null)   return stop();

    return result;
  }

  protected abstract int apply();
  protected abstract int reset();
  protected abstract int start();
  protected abstract int stop();

  public CountersSet getStatistics()
  {
    CountersSet cs = null;
    try {
      if (getStatus() == ServiceInfo.STATUS_RUNNING) cs = mciSme.getStatistics();
    } catch (AdminException e) {
      e.printStackTrace();
    }
    return cs;
  }

  public RuntimeSet getRuntime()
  {
    RuntimeSet rs = null;
    try {
      if (getStatus() == ServiceInfo.STATUS_RUNNING) rs = mciSme.getRuntime();
    } catch (AdminException e) {
      e.printStackTrace();
    }
    return rs;
  }

  public boolean isConfigChanged() {
    return getMCISmeContext().isChangedOptions() ||
           getMCISmeContext().isChangedDrivers() ||
           getMCISmeContext().isChangedTemplates();
  }
  public boolean isOptionsChanged() {
    return getMCISmeContext().isChangedOptions();
  }
  public boolean isDriversChanged() {
    return getMCISmeContext().isChangedDrivers();
  }
  public boolean isTemplatesChanged() {
    return getMCISmeContext().isChangedTemplates();
  }

  public byte getStatus() {
    return ((mciSme != null) ? mciSme.getInfo().getStatus():ServiceInfo.STATUS_UNKNOWN);
  }
  public boolean isMCISmeStarted() {
    return mcismeStarted;
  }

  public String getMbApply() {
    return mbApply;
  }
  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }
  public String getMbReset() {
    return mbReset;
  }
  public void setMbReset(String mbReset) {
    this.mbReset = mbReset;
  }
  public String getMbStart() {
    return mbStart;
  }
  public void setMbStart(String mbStart) {
    this.mbStart = mbStart;
  }
  public String getMbStop() {
    return mbStop;
  }
  public void setMbStop(String mbStop) {
    this.mbStop = mbStop;
  }

}
