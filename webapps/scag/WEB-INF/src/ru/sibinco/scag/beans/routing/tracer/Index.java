package ru.sibinco.scag.beans.routing.tracer;

import org.apache.log4j.Logger;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.endpoints.SmppManager;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


public class Index extends TabledBeanImpl implements TabledBean {

    protected final Logger log = Logger.getLogger(this.getClass());

    public final int traceRouteFound = 10;
    public static final int traceRouteStatus = 20;
    public final int traceRouteNotFound = 30;

    protected int transportId = -1;

    // ************ SMPP **************//
    private String dstAddress = null;
    private String srcAddress = null;
    private String srcSysId = "";
    private String editId = "";
    private String add = "";

    // ************ HTTP **************//
    private int serviceId;
    private int routeId;
    private String abonent;
    private String site;
    private String path = "/";
    private int port = 80;

    private String message = null;
    private List routeInfo = null;
    private LinkedHashMap routeInfoMap = null;
    private List traceResults = null;
    private int messageType = traceRouteStatus;

    private String mbCheck = null;
    private String mbTrace = null;

    protected ScagRoutingManager scagRoutingManager = null;
    protected HttpRoutingManager httpRoutingManager = null;
    protected SmppManager smppManager = null;
    protected Scag scag = null;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        scagRoutingManager = appContext.getScagRoutingManager();
        httpRoutingManager = appContext.getHttpRoutingManager();
        smppManager = appContext.getSmppManager();

        scag = appContext.getScag();
        if (null != mbCheck && (getTransportId() == Transport.SMPP_TRANSPORT_ID)) {
            loadSmppAndCheck();
        } else if (null != mbCheck && (getTransportId() == Transport.HTTP_TRANSPORT_ID)) {
            loadHttpAndCheck();
        } else if (null != mbTrace && null != dstAddress && null != srcAddress
                && (getTransportId() == Transport.SMPP_TRANSPORT_ID)) {
            traceSmppRoute();
        } else if (null != mbTrace && 0 != port
                && (getTransportId() == Transport.HTTP_TRANSPORT_ID)) {
            traceHttpRoute();
        }
    }

    private void loadSmppAndCheck() throws SCAGJspException {
        try {
            traceResults = scag.loadSmppRoutes(scagRoutingManager);
            if (null == traceResults || 0 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = traceRouteStatus;
            routeInfo = null;
            traceResults.remove(0);
            appContext.getStatuses().setSmppRoutesLoaded(true);
        } catch (SibincoException e) {
            e.printStackTrace();
            error("errors.routing.tracer.LoadAndCheckFailed", e.getMessage());
        }
    }

    private void loadHttpAndCheck() throws SCAGJspException {
        try {
            traceResults = scag.loadHttpRoutes(httpRoutingManager);
            if (null == traceResults || 0 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = traceRouteStatus;
            routeInfo = null;
            traceResults.remove(0);
            appContext.getStatuses().setHttpRoutesLoaded(true);
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

    private void traceSmppRoute() throws SCAGJspException {
        try {
            traceResults = appContext.getScag().traceSmppRoute(dstAddress, srcAddress, srcSysId);
            if (null == traceResults || 1 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = message.startsWith("Route found (disabled)") ? traceRouteStatus :
                    message.startsWith("Route not found") ? traceRouteNotFound : traceRouteFound;
            routeInfo = parseRouteInfo((String) traceResults.get(1));
            if (routeInfo != null) {
                routeInfoMap = new LinkedHashMap();

                if (routeInfo.contains("route id")) {
                    if (routeInfo.indexOf("route id") + 1 < routeInfo.size()){
                        String routeId = (String) routeInfo.get(routeInfo.indexOf("route id") + 1);
                        log.debug("Route id: "+routeId);

                        ScagRoutingManager scagRoutingManager = appContext.getScagRoutingManager();
                        ServiceProvidersManager serviceProviderManager = appContext.getServiceProviderManager();

                        Integer serviceId = scagRoutingManager.getServiceIdByRouteId(routeId);
                        log.debug("Service id: "+serviceId);
                        Integer providerId = scagRoutingManager.getProviderIdByServiceId(serviceId.longValue());
                        log.debug("Provider id: "+providerId);

                        Service service = serviceProviderManager.getServiceById(serviceId.longValue());
                        String service_name = service.getName();
                        log.debug("Service name: "+service_name);

                        ServiceProvider sp = (ServiceProvider) serviceProviderManager.getServiceProviders().get(providerId.longValue());
                        String provider_name = sp.getName();
                        log.debug("Provider name: "+provider_name);

                        routeInfoMap.put("provider", "<a href=\"../msag/services/edit.jsp?editId="+providerId+"\">"+provider_name+"</a>");
                        routeInfoMap.put("service", "<a href=\"../msag/services/service/edit.jsp?parentId="+serviceId+"&editChild=true&editId="+providerId+"\">"+service_name+"</a>");
                    }
                }

                for (int i = 0; i < routeInfo.size(); i += 2) {
                    String key = (String) routeInfo.get(i);
                    String value = (String) routeInfo.get(i + 1);
                    routeInfoMap.put(key, value);
                    log.debug("SMPP route info: "+key+" --> "+value);
                }
            }
            traceResults.remove(0);
            traceResults.remove(0);
        } catch (SibincoException e) {
            e.printStackTrace();
            error("errors.routing.tracer.TraceRouteFailed", e.getMessage());

        }
    }


    private void traceHttpRoute() throws SCAGJspException {
        try {
            traceResults = appContext.getScag().traceHttpRoute(serviceId, routeId, (abonent==null)?"":abonent, (site==null)?"":site, path, port);
            if (null == traceResults || 1 >= traceResults.size())
                throw new SibincoException("Transport error, invalid responce.");
            message = (String) traceResults.get(0);
            messageType = message.startsWith("Route found (disabled)") ? traceRouteStatus :
                    message.startsWith("Http route not found") ? traceRouteNotFound : traceRouteFound;
            routeInfo = parseRouteInfo((String) traceResults.get(1));
            if (routeInfo != null) {
                routeInfoMap = new LinkedHashMap();

                if (routeInfo.contains("ServiceId")) {
                    if (routeInfo.indexOf("ServiceId") + 1 < routeInfo.size()){
                        Long serviceId = new Long((String) routeInfo.get(routeInfo.indexOf("ServiceId") + 1));
                        log.debug("Service id: "+routeId);

                        ScagRoutingManager scagRoutingManager = appContext.getScagRoutingManager();
                        ServiceProvidersManager serviceProviderManager = appContext.getServiceProviderManager();

                        Integer providerId = scagRoutingManager.getProviderIdByServiceId(serviceId);
                        log.debug("Provider id: "+providerId);

                        Service service = serviceProviderManager.getServiceById(serviceId);
                        String service_name = service.getName();
                        log.debug("Service name: "+service_name);

                        ServiceProvider sp = (ServiceProvider) serviceProviderManager.getServiceProviders().get(providerId.longValue());
                        String provider_name = sp.getName();
                        log.debug("Provider name: "+provider_name);

                        routeInfoMap.put("Provider", "<a href=\"../msag/services/edit.jsp?editId="+providerId+"\">"+provider_name+"</a>");
                        routeInfoMap.put("Service", "<a href=\"../msag/services/service/edit.jsp?parentId="+serviceId+"&editChild=true&editId="+providerId+"\">"+service_name+"</a>");
                    }
                }

                for (int i = 0; i < routeInfo.size(); i += 2) {
                    String key = (String) routeInfo.get(i);
                    String value = (String) routeInfo.get(i + 1);
                    if (key.equals("ServiceId")) {
                        value = appContext.getServiceProviderManager().getServiceById(new Long(value)).getName();
                    }
                    routeInfoMap.put(key, value);
                    log.debug("HTTP route info: "+key+" --> "+value);
                }
                routeInfoMap.remove("ServiceId");
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

    public LinkedHashMap getRouteInfoMap() {
        return routeInfoMap;
    }

    public int getMessageType() {
        return messageType;
    }

    public String getEditId() {
        return editId;
    }

    public void setEditId(final String editId) {
        this.editId = editId;
    }

    public String getAdd() {
        return add;
    }

    public void setAdd(final String add) {
        this.add = add;
    }

    public Scag getScag() {
        return scag;
    }

    public void setScag(final Scag scag) {
        this.scag = scag;
    }

    public int getServiceId() {
        return serviceId;
    }

    public void setServiceId(final int serviceId) {
        this.serviceId = serviceId;
    }

    public int getRouteId() {
        return routeId;
    }

    public void setRouteId(int routeId) {
        this.routeId = routeId;
    }

    public String getAbonent() {
        return abonent;
    }

    public void setAbonent(final String abonent) {
        this.abonent = abonent;
    }

    public String getSite() {
        return site;
    }

    public void setSite(final String site) {
        this.site = site;
    }

    public String getPath() {
        if(path == null || path.trim().equals(""))
            path = "/";
        return path;
    }

    public void setPath(final String path) {
        this.path = path;
    }

    public int getPort() {
        return port;
    }

    public void setPort(final int port) {
        this.port = port;
    }

    public int getTraceRouteFound() {
        return traceRouteFound;
    }

    public int getTraceRouteNotFound() {
        return traceRouteNotFound;
    }

    public static int getTraceRouteStatus() {
        return traceRouteStatus;
    }

    protected Collection getDataSource() {
        return new ArrayList();
    }

    protected void delete() {
    }

    public int getTransportId() {
        return transportId;
    }

    public void setTransportId(final int transportId) {
        this.transportId = transportId;
    }

    public String[] getTransportIds() {
        return Transport.transportIds;
    }

    public String[] getTransportTitles() {
        return Transport.transportTitles;
    }
}
