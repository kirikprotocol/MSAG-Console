package ru.sibinco.scag.beans.routing.tracer;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.endpoints.SmppManager;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


public class Index extends TabledBeanImpl implements TabledBean {

    public final int TRACE_ROUTE_FOUND = 10;
    public static final int TRACE_ROUTE_STATUS = 20;
    public final int TRACE_ROUTE_NOT_FOUND = 30;

    private String dstAddress = null;
    private String srcAddress = null;
    private String srcSysId = "";
    private String editId = "";
    private String add = "";

    private String message = null;
    private List routeInfo = null;
    private HashMap routeInfoMap = null;
    private List traceResults = null;
    private int messageType = TRACE_ROUTE_STATUS;

    private String mbCheck = null;
    private String mbTrace = null;

    protected ScagRoutingManager scagRoutingManager = null;
    protected SmppManager smppManager = null;
    protected Scag scag = null;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        scagRoutingManager = appContext.getScagRoutingManager();
        smppManager = appContext.getSmppManager();
        scag = appContext.getScag();
        if (null != mbCheck)
            loadAndCheck();
        else if (null != mbTrace && null != dstAddress && null != srcAddress)
            traceRoute();
    }

    private void loadAndCheck() throws SCAGJspException {
        try {
            traceResults = scag.loadRoutes(scagRoutingManager);
            if (null == traceResults || 0 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = TRACE_ROUTE_STATUS;
            routeInfo = null;
            traceResults.remove(0);
            appContext.getStatuses().setRoutesLoaded(true);
        } catch (SibincoException e) {
            e.printStackTrace();
            error("errors.routing.tracer.LoadAndCheckFailed", e.getMessage());
        }
    }

    private String decodeString(final String str) {
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

    private List parseRouteInfo(final String str) {
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

    private void traceRoute() throws SCAGJspException {
        try {
            traceResults = appContext.getScag().traceRoute(dstAddress, srcAddress, srcSysId);
            if (null == traceResults || 1 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = message.startsWith("Route found (disabled)") ? TRACE_ROUTE_STATUS :
                    message.startsWith("Route not found") ? TRACE_ROUTE_NOT_FOUND : TRACE_ROUTE_FOUND;
            routeInfo = parseRouteInfo((String) traceResults.get(1));
            if (routeInfo != null) {
                routeInfoMap = new HashMap();
                for (int i = 0; i < routeInfo.size(); i += 2) {
                    String key = (String) routeInfo.get(i);
                    String value = (String) routeInfo.get(i + 1);
                    routeInfoMap.put(key, value);
                }
            }
            traceResults.remove(0);
            traceResults.remove(0);
        } catch (SibincoException e) {
            e.printStackTrace();
            error("errors.routing.tracer.TraceRouteFailed", e.getMessage());

        }
    }

    /* ------------------------------ Bean Properties --------------------------- */

    public String getDstAddress() {
        return null != dstAddress ? dstAddress : "";
    }

    public void setDstAddress(final String dstAddress) {
        this.dstAddress = null == dstAddress || 0 >= dstAddress.trim().length() ? "" : dstAddress.trim();
    }

    public String getSrcAddress() {
        return null != srcAddress ? srcAddress : "";
    }

    public void setSrcAddress(final String srcAddress) {
        this.srcAddress = null == srcAddress || 0 >= srcAddress.trim().length() ? "" : srcAddress.trim();
    }

    public String getSrcSysId() {
        return null != srcSysId ? srcSysId : "";
    }

    public void setSrcSysId(final String srcSysId) {
        this.srcSysId = null == srcSysId || 0 >= srcSysId.trim().length() ? "" : srcSysId.trim();
    }

    public String getMbCheck() {
        return mbCheck;
    }

    public void setMbCheck(final String mbCheck) {
        this.mbCheck = mbCheck;
    }

    public String getMbTrace() {
        return mbTrace;
    }

    public void setMbTrace(final String mbTrace) {
        this.mbTrace = mbTrace;
    }

    public List getTraceResults() {
        return traceResults;
    }

    public String getTraceMessage() {
        message = null != message ? message.trim() : null;
        return null != message && 0 < message.length() ? message : null;
    }

    public List getRouteInfo() {
        return routeInfo;
    }

    public HashMap getRouteInfoMap() {
        return routeInfoMap;
    }

    public int getMessageType() {
        return messageType;
    }

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        this.editId = editId;
    }

    public String getAdd() {
        return add;
    }

    public void setAdd(String add) {
        this.add = add;
    }

    public Scag getScag() {
        return scag;
    }

    public void setScag(Scag scag) {
        this.scag = scag;
    }

    public int getTRACE_ROUTE_FOUND() {
        return TRACE_ROUTE_FOUND;
    }

    public int getTRACE_ROUTE_NOT_FOUND() {
        return TRACE_ROUTE_NOT_FOUND;
    }

    public static int getTRACE_ROUTE_STATUS() {
        return TRACE_ROUTE_STATUS;
    }

    protected Collection getDataSource() {
        return new ArrayList();
    }

    protected void delete() {
    }
}
