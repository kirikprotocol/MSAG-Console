package ru.novosoft.smsc.jsp.smsc.tracer;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;


/**
 * Created by IntelliJ IDEA.
 * <p/>
 * User: makar
 * <p/>
 * Date: 14.07.2003
 * <p/>
 * Time: 12:11:34
 * <p/>
 * To change this template use Options | File Templates.
 */
public class Index extends IndexBean
{
  public static final int TRACE_ROUTE_FOUND = 10;
  public static final int TRACE_ROUTE_STATUS = 20;
  public static final int TRACE_ROUTE_NOT_FOUND = 30;

  private String dstAddress = null;
  private String srcAddress = null;
  private String srcSysId = "";

  private String message = null;
  private List routeInfo = null;
  private List traceResults = null;
  private int messageType = TRACE_ROUTE_STATUS;

  private String mbCheck = null;
  private String mbTrace = null;

  protected int init(final List errors)
  {
    return super.init(errors);
  }

  public int process(final HttpServletRequest request)
  {
    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (null != mbCheck)
      return loadAndCheck();
    else if (null != mbTrace && null != dstAddress && null != srcAddress)
      return traceRoute();

    return RESULT_OK;
  }

  private int loadAndCheck()
  {
    try {
      traceResults = appContext.getSmsc().loadRoutes(routeSubjectManager);
      if (null == traceResults || 0 >= traceResults.size())
        throw new AdminException("Transport error, invalid responce.");
      message = (String) traceResults.get(0);
      messageType = TRACE_ROUTE_STATUS;
      routeInfo = null;
      traceResults.remove(0);
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.LoadAndCheckFailed, e.getMessage());
    }
    appContext.getStatuses().setRoutesLoaded(true);
    return RESULT_OK;
  }

  private String decodeString(final String str)
  {
    final int strLen = null == str ? 0 : str.length();
    if (0 >= strLen) return str;
    final StringBuffer sb = new StringBuffer(strLen);
    final char[] strBuff = null == str ? new char[0] : str.toCharArray();
    for (int i = 0; i < strLen; i++) {
      if ('#' != strBuff[i])
        sb.append(strBuff[i]);
      else if (i + 1 < strLen && '#' == strBuff[i + 1]) {
        sb.append('#');
        i++;
      } else if (i + 1 < strLen && 'c' == strBuff[i + 1]) {
        sb.append(':');
        i++;
      } else if (i + 1 < strLen && 's' == strBuff[i + 1]) {
        sb.append(';');
        i++;
      } else
        sb.append(strBuff[i]);
    }
    return sb.toString();
  }

  private List parseRouteInfo(final String str)
  {
    if (null == str || 0 >= str.length()) return null;

    final List list = new ArrayList();
    final StringTokenizer st = new StringTokenizer(str, ";");
    while (st.hasMoreTokens()) {
      final String pair = st.nextToken();
      final int idx = pair.indexOf(":");
      if (0 < idx) {
        list.add(decodeString(pair.substring(0, idx)));
        list.add(decodeString(pair.substring(idx + 1)));
      }
    }
    return list;
  }

  private int traceRoute()
  {
    int result = RESULT_OK;
    try {
      final Mask srcAddressMask = new Mask(srcAddress);
      final Mask dstAddressMask = new Mask(dstAddress);
      if (appContext.getSmsc().getAliases().isContainsAlias(srcAddressMask) && appContext.getSmsc().getAliases().get(srcAddressMask.getMask()).isHide())
        result
        = message(SMSCErrors.error.routes.srcAddressIsAlias,
                  srcAddress + " -> " + appContext.getSmsc().getAliases().getAddressByAlias(srcAddressMask).getAddress().getMask());
      if (appContext.getSmsc().getAliases().isContainsAlias(dstAddressMask))
        result
        = warning(SMSCErrors.error.routes.dstAddressIsAlias,
                  dstAddress + " -> " + appContext.getSmsc().getAliases().getAddressByAlias(dstAddressMask).getAddress().getMask());

      traceResults = appContext.getSmsc().traceRoute(dstAddress, srcAddress, srcSysId);
      if (null == traceResults || 1 >= traceResults.size())
        throw new AdminException("Transport error, invalid responce.");
      message = (String) traceResults.get(0);
      messageType = message.startsWith("Route found (disabled)") ? TRACE_ROUTE_STATUS :
                    message.startsWith("Route not found") ? TRACE_ROUTE_NOT_FOUND : TRACE_ROUTE_FOUND;
      routeInfo = parseRouteInfo((String) traceResults.get(1));
      traceResults.remove(0);
      traceResults.remove(0);
      return result;
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.TraceRouteFailed, e.getMessage());
    }
  }

  /* ------------------------------ Bean Properties --------------------------- */

  public String getDstAddress()
  {
    return null != dstAddress ? dstAddress : "";
  }

  public void setDstAddress(final String dstAddress)
  {
    this.dstAddress = null == dstAddress || 0 >= dstAddress.trim().length() ? "" : dstAddress.trim();
  }

  public String getSrcAddress()
  {
    return null != srcAddress ? srcAddress : "";
  }

  public void setSrcAddress(final String srcAddress)
  {
    this.srcAddress = null == srcAddress || 0 >= srcAddress.trim().length() ? "" : srcAddress.trim();
  }

  public String getSrcSysId()
  {
    return null != srcSysId ? srcSysId : "";
  }

  public void setSrcSysId(final String srcSysId)
  {
    this.srcSysId = null == srcSysId || 0 >= srcSysId.trim().length() ? "" : srcSysId.trim();
  }

  public String getMbCheck()
  {
    return mbCheck;
  }

  public void setMbCheck(final String mbCheck)
  {
    this.mbCheck = mbCheck;
  }

  public String getMbTrace()
  {
    return mbTrace;
  }

  public void setMbTrace(final String mbTrace)
  {
    this.mbTrace = mbTrace;
  }

  public List getTraceResults()
  {
    return traceResults;
  }

  public String getTraceMessage()
  {
    message = null != message ? message.trim() : null;
    return null != message && 0 < message.length() ? message : null;
  }

  public List getRouteInfo()
  {
    return routeInfo;
  }

  public int getMessageType()
  {
    return messageType;
  }
}