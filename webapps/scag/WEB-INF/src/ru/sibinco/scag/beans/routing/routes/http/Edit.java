/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.routing.routes.http;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.CancelChildException;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.routing.http.*;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.util.LocaleMessages;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;


/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * <p/>
 * Date: 08.06.2006
 * Time: 11:43:35
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    private String id;
    private boolean enabled = true;
    private String path = "";
    private String parentId;
    private String serviceName = null;

    /**
     * ********** ABONENTS *******************
     */
    private String[] abonAddress = new String[0];
    private String[] abonSubj = new String[0];
    private Set abonSubjsSet = new HashSet();

    /**
     * ********** SITES *******************
     */
    private String[] siteSubj = new String[0];
    private Site[] sites = new Site[0];
    private String[] sitesHost = new String[0];
    private String[] sitesPort = new String[0];
    private String[] pathLinks = new String[0];
    private Set siteSubjsSet = new HashSet();
    private HttpSession session;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        path = request.getContextPath();
        appContext = getAppContext();
        session = request.getSession();
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }

        if (getMbCancel() != null) {
            throw new CancelChildException(new StringBuffer().append(path).
                    append("/services/service/edit.jsp?parentId=").append(getParentId()).
                    append("&editId=").append(appContext.getServiceProviderManager().
                    getServiceProviderByServiceId(Long.decode(getParentId())).
                    getId()).append("&editChild=true").toString());
        }

        if (getEditId() != null) {
            id = getEditId();
        }
        if (getMbSave() != null) {
            super.process(request, response);
            save();
        }
        load(id);
        super.process(request, response);

    }


    protected void load(final String loadId) throws SCAGJspException {

        final HttpRoute route = (HttpRoute) appContext.getHttpRoutingManager().getRoutes().get(getEditId());
        if (null != route) {
            id = route.getName();
            Abonent abonent = route.getAbonent();
            RouteSite routeSite = route.getRouteSite();
            siteSubj = routeSite.getSiteSubjAsString();
            sites = routeSite.getArraySite();
            sitesHost = routeSite.getSiteAsStr();

            abonSubj = abonent.getSubjectsAsStr();
            for (int i = 0; i < abonSubj.length; i++) {
                abonSubjsSet.add(abonSubj[i]);
            }
            for (int i = 0; i < siteSubj.length; i++) {
                siteSubjsSet.add(siteSubj[i]);
            }
            abonAddress = abonent.getAddressAsStr();
            enabled = route.isEnabled();
            if (null != route.getService()) {
                serviceName = route.getService().getName();
            }
        }
    }

    protected void save() throws SCAGJspException {

        final Map routes = appContext.getHttpRoutingManager().getRoutes();
        String messageText = "";

        String dupSite = findDuplicateSite(sites);
        if (dupSite != null)
            throw new SCAGJspException(Constants.errors.routing.sites.FOUND_DUPLICATE_SITE);
        try {
            final Abonent abonent = createAbonent();
            final RouteSite routeSite = createRouteSite();
            final Service serviceObj = appContext.getServiceProviderManager().getServiceById(Long.decode(getParentId()));
            if (isAdd()) {
                if (routes.containsKey(id))
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, id);
                routes.put(id, new HttpRoute(id, serviceObj, enabled, abonent, routeSite));
                messageText = "Added new route: "  + id + " ";
            } else {
                if (!getEditId().equals(id) && routes.containsKey(id))
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, id);
                routes.remove(getEditId());
                routes.put(id, new HttpRoute(id, serviceObj, enabled, abonent, routeSite));
                messageText = "Changed route: " + id + " ";

            }
        } catch (SibincoException e) {
            logger.error("Could not create new route ", e);
            throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE, e);
        }
        appContext.getHttpRoutingManager().setRoutesChanged(true);
        appContext.getHttpRoutingManager().setRoutesSaved(true);

        StatMessage message = new StatMessage(super.getLoginedPrincipal().getName(), "Routes", messageText);
        appContext.getHttpRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);

        throw new CancelChildException(new StringBuffer().append(path).append("/services/service/edit.jsp?parentId=").
                append(getParentId()).append("&editId=").append(
                appContext.getServiceProviderManager().getServiceProviderByServiceId(
                        Long.decode(getParentId())).getId()).append("&editChild=true").toString());
    }


    private RouteSite createRouteSite() throws SibincoException {
        RouteSite routeSite = new RouteSite();

        for (int i = 0; i < siteSubj.length; i++) {
            final String subjectName = siteSubj[i];
            final HttpSite httpSite = (HttpSite) appContext.getHttpRoutingManager().getSites().get(subjectName);
            if (null != httpSite) {
                routeSite.getSiteSubjects().put(httpSite.getName(), httpSite);
            }
        }
        for (int i = 0; i < sitesHost.length; i++) {
            for (int j = 0; j < sitesPort.length; j++) {
                String port = sitesPort[j];
                int portlen = sitesPort[j].lastIndexOf('_');
                String siteName = port.substring(0, portlen);
                port = port.substring(portlen + 1);
                if (sitesHost[i].equals(siteName)) {
                    final Site site = new Site(sitesHost[i], Integer.parseInt(port));
                    List listPath = new ArrayList();
                    for (int k = 0; k < pathLinks.length; k++) {
                        String pathLink = pathLinks[k];
                        int pathlen = pathLink.lastIndexOf('_');
                        String sitPath = pathLink.substring(0, pathlen);
                        if (sitPath.equals(site.getHost())) {
                            listPath.add(pathLink.substring(pathlen + 1));
                        }
                    }
                    site.setPathLinks((String[]) listPath.toArray(new String[listPath.size()]));
                    routeSite.getSites().put(site.getHost(), site);
                }
            }
        }
        return routeSite;
    }

    public String findDuplicateSite(final Site[] sites) {
        Map collide = new HashMap();
        for (int i = 0; i < sites.length; ++i) {
            String key = sites[i].getHost();
            if (collide.containsKey(key))
                return sites[i].getHost();
            else
                collide.put(key, null);
        }

        return null;
    }

    private Abonent createAbonent() throws SibincoException {
        Abonent abonent = new Abonent();
        for (int i = 0; i < abonSubj.length; i++) {
            final String subjectName = abonSubj[i];
            final HttpSubject httpSubject = (HttpSubject) appContext.getHttpRoutingManager().getSubjects().get(subjectName);
            if (null != httpSubject) {
                abonent.getSubjects().put(httpSubject.getName(), httpSubject);
            }
        }
        for (int i = 0; i < abonAddress.length; i++) {
            String abonAddres = abonAddress[i];
            if (null != abonAddres && 0 < abonAddres.trim().length()) {
                final Mask mask = new Mask(abonAddres);
                abonent.getAddress().put(mask.getMask(), mask);
            }
        }
        return abonent;
    }

    public List getAllUncheckedAbonSubjects() {
        final SortedList abonSubjs = new SortedList(appContext.getHttpRoutingManager().getSubjects().keySet());
        abonSubjs.removeAll(abonSubjsSet);
        return abonSubjs;
    }

    public List getAllUncheckedSiteSubject() {
        final SortedList siteSubj = new SortedList(appContext.getHttpRoutingManager().getSites().keySet());
        siteSubj.removeAll(siteSubjsSet);
        return siteSubj;
    }

    public String getId() {
        return id;
    }

    public void setId(final String id) {
        this.id = id;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public String getPath() {
        return path;
    }

    public void setPath(final String path) {
        this.path = path;
    }

    public String getParentId() {
        return parentId;
    }

    public void setParentId(final String parentId) {
        this.parentId = parentId;
    }

    public String getServiceName() {
        return serviceName;
    }

    public void setServiceName(final String serviceName) {
        this.serviceName = serviceName;
    }

    public String[] getAbonAddress() {
        return abonAddress;
    }

    public void setAbonAddress(String[] abonAddress) {
        this.abonAddress = abonAddress;
    }

    public String[] getAbonSubj() {
        return abonSubj;
    }

    public void setAbonSubj(final String[] abonSubj) {
        this.abonSubj = abonSubj;
    }

    public String[] getSiteSubj() {
        return siteSubj;
    }

    public void setSiteSubj(final String[] siteSubj) {
        this.siteSubj = siteSubj;
    }

    public Site[] getSites() {
        return sites;
    }

    public void setSites(final Site[] sites) {
        this.sites = sites;
    }

    public HttpSession getSession() {
        return session;
    }

    public String[] getPathLinks() {
        return pathLinks;
    }

    public void setPathLinks(String[] pathLinks) {
        this.pathLinks = pathLinks;
    }

    public String[] getSitesHost() {
        return sitesHost;
    }

    public void setSitesHost(String[] sitesHost) {
        this.sitesHost = sitesHost;
    }

    public String[] getSitesPort() {
        return sitesPort;
    }

    public void setSitesPort(String[] sitesPort) {
        this.sitesPort = sitesPort;
    }

    public String[] getSiteSubjAsString(final Map siteSubjects) {
        List subjSiteList = new ArrayList();
        for (Iterator i = siteSubjects.values().iterator(); i.hasNext();) {
            final HttpSubject httpSubject = (HttpSubject) i.next();
            subjSiteList.add(httpSubject.getName());
        }
        return (String[]) subjSiteList.toArray(new String[subjSiteList.size()]);
    }
}
