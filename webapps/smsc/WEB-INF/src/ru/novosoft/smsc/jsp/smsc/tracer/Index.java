package ru.novosoft.smsc.jsp.smsc.tracer;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 14.07.2003
 * Time: 12:11:34
 * To change this template use Options | File Templates.
 */
public class Index extends IndexBean
{
  private String dstAddress = null;
  private String srcAddress = null;
  private String srcSysId   = null;

  private String traceResults = null;

  private String mbCheck = null;
  private String mbTrace = null;

  protected int init(List errors)
  {
    return super.init(errors);
  }
  public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbCheck != null)
      return loadAndCheck();
    else if (mbTrace != null)
      return traceRoute();

    return RESULT_OK;
  }

  private int loadAndCheck()
  {
    try
    {
      if (!appContext.getStatuses().isRoutesSaved()) routeSubjectManager.save();
      traceResults = appContext.getSmsc().loadRoutes();
      appContext.getStatuses().setRoutesSaved(true);
    }
    catch (AdminException e) {
      return error(SMSCErrors.error.routes.LoadAndCheckFailed, e.getMessage());
    }
    return RESULT_OK;
  }

  private int traceRoute()
  {
    try
    {
      if (!appContext.getStatuses().isRoutesSaved()) routeSubjectManager.save();
      traceResults = appContext.getSmsc().traceRoute(dstAddress.trim(), srcAddress.trim(), srcSysId.trim());
      appContext.getStatuses().setRoutesSaved(true);
    }
    catch (AdminException e) {
      return error(SMSCErrors.error.routes.TraceRouteFailed, e.getMessage());
    }
    return RESULT_OK;
  }

  /* ------------------------------ Bean Properties --------------------------- */

  public String getDstAddress() {
    return (dstAddress != null) ? dstAddress:"";
  }
  public void setDstAddress(String dstAddress) {
    this.dstAddress = dstAddress;
  }

  public String getSrcAddress() {
    return (srcAddress != null) ? srcAddress:"";
  }
  public void setSrcAddress(String srcAddress) {
    this.srcAddress = srcAddress;
  }

  public String getSrcSysId() {
    return (srcSysId != null) ? srcSysId:"";
  }
  public void setSrcSysId(String srcSysId) {
    this.srcSysId = srcSysId;
  }

  public String getMbCheck() {
    return mbCheck;
  }
  public void setMbCheck(String mbCheck) {
    this.mbCheck = mbCheck;
  }

  public String getMbTrace() {
    return mbTrace;
  }
  public void setMbTrace(String mbTrace) {
    this.mbTrace = mbTrace;
  }

  public String getTraceResults() {
    traceResults = (traceResults != null) ? traceResults.trim():null;
    return (traceResults != null && traceResults.length() > 0) ? traceResults:null;
  }
}
