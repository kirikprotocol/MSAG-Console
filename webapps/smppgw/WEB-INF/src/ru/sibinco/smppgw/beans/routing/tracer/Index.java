package ru.sibinco.smppgw.beans.routing.tracer;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.TabledBeanImpl;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import ru.sibinco.smppgw.backend.sme.GwSmeManager;
import ru.sibinco.smppgw.backend.Smppgw;
import ru.sibinco.smppgw.backend.Gateway;
import ru.sibinco.smppgw.backend.protocol.alias.Alias;
import ru.sibinco.smppgw.Constants;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


/**
 * Created by andrey Date: 17.03.2005 Time: 17:39:13
 */
public class Index extends TabledBeanImpl implements TabledBean
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

    protected GwRoutingManager gwRoutingManager = null;
    protected GwSmeManager gwSmeManager = null;
    protected Smppgw smppgw = null;
    protected Gateway gateway = null;

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SmppgwJspException
    {
      super.process(request,response);
      gwRoutingManager = appContext.getGwRoutingManager();
      gwSmeManager = appContext.getGwSmeManager();
      smppgw=appContext.getSmppgw();
      if (null != mbCheck)
         loadAndCheck();
      else if (null != mbTrace && null != dstAddress && null != srcAddress)
        traceRoute();

    }

    private void loadAndCheck() throws SmppgwJspException
    {
      try {
        traceResults = smppgw.loadRoutes(gwRoutingManager);
        if (null == traceResults || 0 >= traceResults.size())
          throw new SibincoException("Transport error, invalid responce.");
        message = (String) traceResults.get(0);
        messageType = TRACE_ROUTE_STATUS;
        routeInfo = null;
        traceResults.remove(0);
      } catch (SibincoException e) {  e.printStackTrace();
        throw new SmppgwJspException(Constants.errors.routing.tracer.LoadAndCheckFailed,e.getMessage() );
      }
      appContext.getStatuses().setRoutesLoaded(true);
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
        }
        else if (i + 1 < strLen && 'c' == strBuff[i + 1]) {
          sb.append(':');
          i++;
        }
        else if (i + 1 < strLen && 's' == strBuff[i + 1]) {
          sb.append(';');
          i++;
        }
        else
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

    private void traceRoute() throws SmppgwJspException
    {
      try {
        final Mask srcAddressMask = new Mask(srcAddress);
        final Mask dstAddressMask = new Mask(dstAddress);
        if (appContext.getSmppgw().getAliases().isContainsAlias(srcAddressMask)) {
          final Alias a = appContext.getSmppgw().getAliases().getAliasByAddress(srcAddressMask);
          if (a != null && a.isHide())
             throw new SibincoException(Constants.errors.routing.tracer.srcAddressIsAlias
                     +srcAddress + " -> " + appContext.getSmppgw().getAliases().getAddressByAlias(srcAddressMask).getAddress().getMask());
        }
        if (appContext.getSmppgw().getAliases().isContainsAlias(dstAddressMask))
         throw new SibincoException(Constants.errors.routing.tracer.srcAddressIsAlias
                    +dstAddress + " -> " + appContext.getSmppgw().getAliases().getAddressByAlias(dstAddressMask).getAddress().getMask());

       traceResults = appContext.getSmppgw().traceRoute(dstAddress, srcAddress, srcSysId);
        if (null == traceResults || 1 >= traceResults.size())
          throw new SibincoException("Transport error, invalid responce.");
        message = (String) traceResults.get(0);
        messageType = message.startsWith("Route found (disabled)") ? TRACE_ROUTE_STATUS :
                message.startsWith("Route not found") ? TRACE_ROUTE_NOT_FOUND : TRACE_ROUTE_FOUND;
        routeInfo = parseRouteInfo((String) traceResults.get(1));
        traceResults.remove(0);
        traceResults.remove(0);
      } catch (SibincoException e) { e.printStackTrace();
        throw new SmppgwJspException(Constants.errors.routing.tracer.TraceRouteFailed,e.getMessage() );

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

  public Gateway getGateway()
  {
    return gateway;
  }

  public void setGateway(Gateway gateway)
  {
    this.gateway = gateway;
  }

   protected Collection getDataSource()
  {
    return new ArrayList();//appContext.getGwRoutingManager().getRoutes().values();
  }

  protected void delete()
  {
   // appContext.getGwRoutingManager().getRoutes().keySet().removeAll(checkedSet);
   // appContext.getStatuses().setRoutesChanged(true);
  }
}
