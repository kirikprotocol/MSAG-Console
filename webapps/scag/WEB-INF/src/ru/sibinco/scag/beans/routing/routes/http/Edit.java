/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.routing.routes.http;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.routing.http.*;
import ru.sibinco.scag.backend.routing.http.placement.AbonentsPlacement;
import ru.sibinco.scag.backend.routing.http.placement.SitePlacement;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.CancelChildException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

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

    public static final String ADDRESS_SELECT = "addressSelect_";
    public static final String SITE_SELECT = "siteSelect_";
    public static final String ABON_USR_SELECT = "abonUsrSelect_";
    public static final String SITE_USR_SELECT = "siteUsrSelect_";
    public static final String SITE_ROUTE_ID_SELECT = "siteRouteIdSelect_";
    public static final String SITE_SERVICE_ID_SELECT = "siteServiceIdSelect_";

    public static final String URL_TYPE = "url";
    public static final String PARAM_TYPE = "param";
    public static final String HEADER_TYPE = "header";
    public static final String COOKIE_TYPE = "cookie";

    private String id;
    private String name;
    private boolean enabled;
    private boolean defaultRoute;
    private boolean transit;
    private boolean saa;
    private String path = "";
    private String parentId;
    private String serviceName = null;
    private int ton = 1;
    private int npi = 1;


    /**
     * ********** ABONENTS *******************
     */
    private String[] abonAddress = new String[0];
    private String[] abonSubj = new String[0];
    private Set abonSubjsSet = new HashSet();

    private AbonentsPlacement[] abonentUsr = new AbonentsPlacement[0];
    private String[] abonentUsrName = new String[0];
    private String[] abonentUsrType = new String[0];

    /**
     * ********** SITES *******************
     */
    private String[] siteSubj = new String[0];
    private Site[] sites = new Site[0];
    private String[] sitesHost = new String[0];
    private String[] sitesPort = new String[0];
    private String[] pathLinks = new String[0];
    private String defaultSiteObjId = null;
    private Set siteSubjsSet = new HashSet();

    private SitePlacement[] siteUsr = new SitePlacement[0];
    private String[] siteUsrName = new String[0];
    private String[] siteUsrType = new String[0];

    private SitePlacement[] siteAddress = new SitePlacement[0];
    private String[] siteAddrName = new String[0];
    private String[] siteAddrType = new String[0];

    private SitePlacement[] siteRouteId = new SitePlacement[0];
    private String[] siteRouteIdName = new String[0];
    private String[] siteRouteIdType = new String[0];

    private SitePlacement[] siteServiceId = new SitePlacement[0];
    private String[] siteServiceIdName = new String[0];
    private String[] siteServiceIdType = new String[0];

    private String[] optionTypes = {URL_TYPE, PARAM_TYPE, HEADER_TYPE, COOKIE_TYPE};

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

            //SITE ADDRESS
            List siteAddrList = new ArrayList();
            for (int i = 0; i < siteAddrName.length; i++) {
                String s = new StringBuffer().append(SITE_SELECT).append(siteAddrName[i]).append(siteAddrType[i].
                        substring(siteAddrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);
                for (int j = 0; j < results.length; j++) {
                    try {
                        siteAddrList.add(new SitePlacement(results[j], siteAddrName[i]));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }
                }
            }
            siteAddress = (SitePlacement[]) siteAddrList.toArray(new SitePlacement[siteAddrList.size()]);
            siteAddrList.clear();

            //ABONENT USR
            List abonentUsrList = new ArrayList();
            for (int i = 0; i < abonentUsrName.length; i++) {
                String s = new StringBuffer().append(ABON_USR_SELECT).append(abonentUsrName[i]).
                        append(abonentUsrType[i].substring(abonentUsrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);

                int priority = i + 1;
                for (int j = 0; j < results.length; j++) {
                    try {
                        abonentUsrList.add(new AbonentsPlacement(results[j], abonentUsrName[i], priority));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }

                }
            }
            abonentUsr = (AbonentsPlacement[]) abonentUsrList.toArray(new AbonentsPlacement[abonentUsrList.size()]);
            abonentUsrList.clear();

            //SITE USR
            List siteUsrrList = new ArrayList();
            for (int i = 0; i < siteUsrName.length; i++) {
                String s = new StringBuffer().append(SITE_USR_SELECT).append(siteUsrName[i]).append(siteUsrType[i].
                        substring(siteUsrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);
                for (int j = 0; j < results.length; j++) {
                    try {
                        siteUsrrList.add(new SitePlacement(results[j], siteUsrName[i]));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }
                }
            }
            siteUsr = (SitePlacement[]) siteUsrrList.toArray(new SitePlacement[siteUsrrList.size()]);
            siteAddrList.clear();

          //**********************************************************************************//
          //                            DEFAULT SITE ROUTE ID PLACE                           //
          //**********************************************************************************//
          List siteRouteIdList = new ArrayList();
          for (int i = 0; i < siteRouteIdName.length; i++) {
              String s = new StringBuffer().append(SITE_ROUTE_ID_SELECT).append(siteRouteIdName[i]).append(siteRouteIdType[i].
                      substring(siteRouteIdType[i].lastIndexOf('_'))).toString();
              String[] results = request.getParameterValues(s);
              for (int j = 0; j < results.length; j++) {
                  try {
                      siteRouteIdList.add(new SitePlacement(results[j], siteRouteIdName[i]));
                  } catch (SibincoException e) {
                      e.printStackTrace();
                  }
              }
          }
          siteRouteId = (SitePlacement[]) siteRouteIdList.toArray(new SitePlacement[siteRouteIdList.size()]);
          siteRouteIdList.clear();

          //**********************************************************************************//
          //                            DEFAULT SITE SERVICE ID PLACE                         //
          //**********************************************************************************//
          List siteServiceIdList = new ArrayList();
          for (int i = 0; i < siteServiceIdName.length; i++) {
              String s = new StringBuffer().append(SITE_SERVICE_ID_SELECT).append(siteServiceIdName[i]).append(siteServiceIdType[i].
                      substring(siteServiceIdType[i].lastIndexOf('_'))).toString();
              String[] results = request.getParameterValues(s);
              for (int j = 0; j < results.length; j++) {
                  try {
                      siteServiceIdList.add(new SitePlacement(results[j], siteServiceIdName[i]));
                  } catch (SibincoException e) {
                      e.printStackTrace();
                  }
              }
          }
          siteServiceId = (SitePlacement[]) siteServiceIdList.toArray(new SitePlacement[siteServiceIdList.size()]);
          siteServiceIdList.clear();

            saveRoutes();
        }
        load(id);
        super.process(request, response);

    }


    protected void load(final String loadId) throws SCAGJspException {

        final HttpRoute route = (HttpRoute) appContext.getHttpRoutingManager().getRoutes().get(getEditId());
        if (null != route) {
            id = route.getId().toString();
            name = route.getName();
            Abonent abonent = route.getAbonent();
            RouteSite routeSite = route.getRouteSite();
            siteSubj = routeSite.getSiteSubjAsString();
            sites = routeSite.getArraySite();
            sitesHost = routeSite.getSiteAsStr();
            defaultSiteObjId = routeSite.getDefaultSiteObjId();
            ton = abonent.getTon();
            npi = abonent.getNpi();
            abonSubj = abonent.getSubjectsAsStr();
            for (int i = 0; i < abonSubj.length; i++) {
                abonSubjsSet.add(abonSubj[i]);
            }
            for (int i = 0; i < siteSubj.length; i++) {
                siteSubjsSet.add(siteSubj[i]);
            }
            abonAddress = abonent.getAddressAsStr();
            enabled = route.isEnabled();
            defaultRoute = route.isDefaultRoute();
            transit = route.isTransit();
            saa = route.isSaa();
            if (null != route.getService()) {
                serviceName = route.getService().getName();
            }
            abonentUsr = getAbonentsPlacements(route.getAbonent().getAbonUsrPlace());
            siteUsr = getSitePlacement(route.getRouteSite().getUsrPlace());
            siteAddress = getSitePlacement(route.getRouteSite().getAddressPlace());
            siteRouteId = getSitePlacement(route.getRouteSite().getRouteIdPlace());
            siteServiceId = getSitePlacement(route.getRouteSite().getServiceIdPlace());
        }
        if (isAdd()) {
            enabled = true;
            defaultRoute = appContext.getHttpRoutingManager().isDefaultRoute(Long.decode(getParentId()));
        }
    }

    protected void saveRoutes() throws SCAGJspException {

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
                HttpRoute route = appContext.getHttpRoutingManager().createRoute(
                        name, serviceObj, enabled, defaultRoute, transit, abonent, routeSite, saa);
                routes.put(route.getId().toString(), route);
                messageText = "Added new route: " + name + " ";
            } else {
                if (!getEditId().equals(id) && routes.containsKey(id))
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, id);
                routes.remove(getEditId());
                routes.put(id, new HttpRoute(Long.valueOf(id), name, serviceObj, enabled, defaultRoute,
                        transit, abonent, routeSite, saa));
                messageText = "Changed route: " + name + " ";

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

    protected void save() throws SCAGJspException {
    }


    private RouteSite createRouteSite() throws SibincoException {
        RouteSite routeSite = new RouteSite();
        routeSite.setDefaultSiteObjId(defaultSiteObjId);

        for (int i = 0; i < siteSubj.length; i++) {
            final String subjectName = siteSubj[i];
            final HttpSite httpSite = (HttpSite) appContext.getHttpRoutingManager().getSites().get(subjectName);
            if (null != httpSite) {
                routeSite.getSiteSubjects().put(httpSite.getName(), httpSite);
            }
        }
        //for (int i = 0; i < sitesHost.length; i++) {
            for (int j = 0; j < sitesPort.length; j++) {
                String port = sitesPort[j];
                int portlen = sitesPort[j].lastIndexOf(':');
                String siteName = port.substring(0, portlen);
                port = port.substring(portlen + 1);
                //if (sitesHost[i].equals(siteName)) {
                    final Site site = new Site(siteName, Integer.parseInt(port), (siteName+":"+port).equals(defaultSiteObjId));
                    List listPath = new ArrayList();
                    for (int k = 0; k < pathLinks.length; k++) {
                        String pathLink = pathLinks[k];
                        int s1 = pathLink.lastIndexOf(':');
                        String sitPath = pathLink.substring(0, s1);
                        int s2 = pathLink.lastIndexOf("^");
                        if (sitPath.equals(site.getHost())) {
                            String path = pathLink.substring(s2 + 1);
                            if (port.equals(pathLink.substring(s1+1, s2))){
                                listPath.add(path);
                                logger.debug("site: host="+sitPath + ", port="+port+", path="+path);
                            }
                        }
                    }
                    site.setPathLinks((String[]) listPath.toArray(new String[listPath.size()]));
                    routeSite.getSites().put(site.getId(), site);
                //}
            }
            routeSite.getAddressPlace().clear();
            routeSite.getUsrPlace().clear();
            routeSite.getRouteIdPlace().clear();
            routeSite.getServiceIdPlace().clear();

            for (int j = 0; j < siteAddress.length; j++) {
                routeSite.getAddressPlace().add(siteAddress[j]);
            }
            for (int j = 0; j < siteUsr.length; j++) {
                routeSite.getUsrPlace().add(siteUsr[j]);
            }
            for (int j = 0; j < siteRouteId.length; j++) {
                routeSite.getRouteIdPlace().add(siteRouteId[j]);
            }
            for (int j = 0; j < siteServiceId.length; j++) {
                routeSite.getServiceIdPlace().add(siteServiceId[j]);
            }

        //}
        return routeSite;
    }

    public String findDuplicateSite(final Site[] sites) {
        Map collide = new HashMap();
        for (int i = 0; i < sites.length; ++i) {
            String key = sites[i].getId();
            if (collide.containsKey(key))
                return sites[i].getId();
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
            abonent.getAddress().put(abonAddres, abonAddres);
        }
        abonent.setNpi(getNpi());
        abonent.setTon(getTon());
        abonent.getAbonUsrPlace().clear();
        for (int i = 0; i < abonentUsr.length; i++) {
            abonent.getAbonUsrPlace().add(abonentUsr[i]);
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

    private AbonentsPlacement[] getAbonentsPlacements(List addressPlace) {
            return (AbonentsPlacement[]) addressPlace.toArray(new AbonentsPlacement[addressPlace.size()]);
        }

        private SitePlacement[] getSitePlacement(List addressPlace) {
            return (SitePlacement[]) addressPlace.toArray(new SitePlacement[addressPlace.size()]);
        }


    public String getId() {
        return id;
    }

    public void setId(final String id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isDefaultRoute() {
        return defaultRoute;
    }


    public void setDefaultRoute(final boolean defaultRoute) {
        this.defaultRoute = defaultRoute;
    }

    public boolean isTransit() {
        return transit;
    }

    public void setTransit(final boolean transit) {
        this.transit = transit;
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

    public String getDefaultSiteObjId() {
        return defaultSiteObjId;
    }

    public void setDefaultSiteObjId(String defaultSiteObjId) {
        this.defaultSiteObjId = defaultSiteObjId;
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

    public int getTon() {
        return ton;
    }

    public void setTon(final int ton) {
        this.ton = ton;
    }

    public int getNpi() {
        return npi;
    }

    public void setNpi(final int npi) {
        this.npi = npi;
    }

    //ABONENT USR
    public AbonentsPlacement[] getAbonentUsr() {
        return abonentUsr;
    }

    public void setAbonentUsr(AbonentsPlacement[] abonentUsr) {
        this.abonentUsr = abonentUsr;
    }

    public String[] getAbonentUsrName() {
        return abonentUsrName;
    }

    public void setAbonentUsrName(String[] abonentUsrName) {
        this.abonentUsrName = abonentUsrName;
    }

    public String[] getAbonentUsrType() {
        return abonentUsrType;
    }

    public void setAbonentUsrType(String[] abonentUsrType) {
        this.abonentUsrType = abonentUsrType;
    }

    //SITE USR
    public SitePlacement[] getSiteUsr() {
        return siteUsr;
    }

    public void setSiteUsr(SitePlacement[] siteUsr) {
        this.siteUsr = siteUsr;
    }

    public String[] getSiteUsrName() {
        return siteUsrName;
    }

    public void setSiteUsrName(String[] siteUsrName) {
        this.siteUsrName = siteUsrName;
    }

    public String[] getSiteUsrType() {
        return siteUsrType;
    }

    public void setSiteUsrType(String[] siteUsrType) {
        this.siteUsrType = siteUsrType;
    }

    public String[] getOptionTypes() {
        return optionTypes;
    }

    //SITE ADDRESS
    public SitePlacement[] getSiteAddress() {
        return siteAddress;
    }

    public void setSiteAddress(SitePlacement[] siteAddress) {
        this.siteAddress = siteAddress;
    }

    public String[] getSiteAddrName() {
        return siteAddrName;
    }

    public void setSiteAddrName(String[] siteAddrName) {
        this.siteAddrName = siteAddrName;
    }

    public String[] getSiteAddrType() {
        return siteAddrType;
    }

    public void setSiteAddrType(String[] siteAddrType) {
        this.siteAddrType = siteAddrType;
    }
    //SITE ROUTE ID
    public SitePlacement[] getSiteRouteId() {
        return siteRouteId;
    }

    public void setSiteRouteId(SitePlacement[] siteRouteId) {
        this.siteRouteId = siteRouteId;
    }

    public String[] getSiteRouteIdName() {
        return siteRouteIdName;
    }

    public void setSiteRouteIdName(String[] siteRouteIdName) {
        this.siteRouteIdName = siteRouteIdName;
    }

    public String[] getSiteRouteIdType() {
        return siteRouteIdType;
    }

    public void setSiteRouteIdType(String[] siteRouteIdType) {
        this.siteRouteIdType = siteRouteIdType;
    }
    //SITE SERVICE ID
    public SitePlacement[] getSiteServiceId() {
        return siteServiceId;
    }

    public void setSiteServiceId(SitePlacement[] siteServiceId) {
        this.siteServiceId = siteServiceId;
    }

    public String[] getSiteServiceIdName() {
        return siteServiceIdName;
    }

    public void setSiteServiceIdName(String[] siteServiceIdName) {
        this.siteServiceIdName = siteServiceIdName;
    }

    public String[] getSiteServiceIdType() {
        return siteServiceIdType;
    }

    public void setSiteServiceIdType(String[] siteServiceIdType) {
        this.siteServiceIdType = siteServiceIdType;
    }

    public boolean isSaa() {
        return saa;
    }

    public void setSaa(boolean saa) {
        this.saa = saa;
    }
}
