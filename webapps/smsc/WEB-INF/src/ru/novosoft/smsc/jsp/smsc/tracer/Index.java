package ru.novosoft.smsc.jsp.smsc.tracer;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.StringTokenizer;
import java.util.ArrayList;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 14.07.2003
 * Time: 12:11:34
 * To change this template use Options | File Templates.
 */
public class Index extends IndexBean
{
  public final static int TRACE_ROUTE_FOUND     = 10;
  public final static int TRACE_ROUTE_STATUS    = 20;
  public final static int TRACE_ROUTE_NOT_FOUND = 30;

  private String dstAddress = null;
  private String srcAddress = null;
  private String srcSysId   = "";

  private String message = null;
  private List   routeInfo = null;
  private List   traceResults = null;
  private int    messageType = TRACE_ROUTE_STATUS;

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
    else if (mbTrace != null && dstAddress != null && srcAddress != null)
      return traceRoute();

    return RESULT_OK;
  }

  private int loadAndCheck()
  {
    try
    {
      if (!appContext.getStatuses().isRoutesSaved()) {
        routeSubjectManager.save();
        appContext.getStatuses().setRoutesSaved(true);
      }

      traceResults = appContext.getSmsc().loadRoutes();
      if (traceResults == null || traceResults.size() <= 0)
        throw new AdminException("Transport error, invalid responce.");
      message = (String)traceResults.get(0);
      messageType = TRACE_ROUTE_STATUS;
      routeInfo = null;
      traceResults.remove(0);
    }
    catch (AdminException e) {
      return error(SMSCErrors.error.routes.LoadAndCheckFailed, e.getMessage());
    }
    return RESULT_OK;
  }

  private List parseRouteInfo(String str)
  {
    if (str == null || str.length() <= 0) return null;

    ArrayList list = new ArrayList();
    StringTokenizer st = new StringTokenizer(str, ";");
    while (st.hasMoreTokens()) {
      String pair = st.nextToken();
      int idx = pair.indexOf(":");
      list.add(pair.substring(0, idx));
      list.add(pair.substring(idx+1));

    }
    return list;
  }

  private int traceRoute()
  {
    try
    {
      if (!appContext.getStatuses().isRoutesSaved()) {
        routeSubjectManager.save();
        appContext.getStatuses().setRoutesSaved(true);
      }
      traceResults = appContext.getSmsc().traceRoute(dstAddress, srcAddress, srcSysId);
      if (traceResults == null || traceResults.size() <= 1)
        throw new AdminException("Transport error, invalid responce.");
      message = (String)traceResults.get(0);
      messageType = (message.startsWith("Route found")) ? TRACE_ROUTE_FOUND :
                      ((message.startsWith("Route not found")) ? TRACE_ROUTE_NOT_FOUND : TRACE_ROUTE_STATUS);
      routeInfo = parseRouteInfo((String)traceResults.get(1));
      traceResults.remove(0); traceResults.remove(0);
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
    this.dstAddress = (dstAddress == null || dstAddress.trim().length() <= 0) ? "":dstAddress.trim();
  }

  public String getSrcAddress() {
    return (srcAddress != null) ? srcAddress:"";
  }
  public void setSrcAddress(String srcAddress) {
    this.srcAddress = (srcAddress == null || srcAddress.trim().length() <= 0) ? "":srcAddress.trim();
  }

  public String getSrcSysId() {
    return (srcSysId != null) ? srcSysId:"";
  }
  public void setSrcSysId(String srcSysId) {
    this.srcSysId = (srcSysId == null || srcSysId.trim().length() <= 0) ? "":srcSysId.trim();
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

  public List getTraceResults() {
    return traceResults;
  }
  public String getTraceMessage() {
    message = (message != null) ? message.trim():null;
    return (message != null && message.length() > 0) ? message:null;
  }
  public List getRouteInfo() {
    return routeInfo;
  }
  public int getMessageType() {
    return messageType;
  }
}
