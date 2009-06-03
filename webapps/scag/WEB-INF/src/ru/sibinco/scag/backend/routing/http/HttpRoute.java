/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;

import java.io.PrintWriter;
import java.util.Map;

/**
 * The <code>HttpRoute</code> class represents
 * <p><p/>
 * Date: 06.05.2006
 * Time: 15:17:58
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpRoute {

    private final Logger logger = Logger.getLogger(this.getClass());

    private Long id;
    private String name;
    private Service service;
    private boolean enabled = false;
    private boolean defaultRoute = false;
    private boolean transit = false;
    private boolean saa = true;

    private Abonent abonent;
    private RouteSite routeSite;

    public HttpRoute(final Long id, final String routeName, final Service service, final boolean enabled, final boolean defaultRoute, final boolean transit,
                     final Abonent abonent, final RouteSite routeSite) {
        if (routeName == null)
            throw new NullPointerException("Http route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Http route name is too long");
        if (abonent == null)
            throw new NullPointerException("Abonents list is null");
        if (routeSite == null)
            throw new NullPointerException("Sites list is null");

        this.id = id;
        this.name = routeName;
        this.service = service;
        this.enabled = enabled;
        this.defaultRoute = defaultRoute;
        this.transit = transit;
        this.abonent = abonent;
        this.routeSite = routeSite;
    }

    public HttpRoute(final Long id, final String routeName, final Service service, final boolean enabled, final boolean defaultRoute, final boolean transit,
                     final Abonent abonent, final RouteSite routeSite, boolean saa) {
        if (routeName == null)
            throw new NullPointerException("Http route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Http route name is too long");
        if (abonent == null)
            throw new NullPointerException("Abonents list is null");
        if (routeSite == null)
            throw new NullPointerException("Sites list is null");

        this.id = id;
        this.name = routeName;
        this.service = service;
        this.enabled = enabled;
        this.defaultRoute = defaultRoute;
        this.transit = transit;
        this.abonent = abonent;
        this.routeSite = routeSite;
        this.saa = saa;
    }

    public HttpRoute(Long id, String routeName) {
        if (routeName == null)
            throw new NullPointerException("Http route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Http route name is too long");
        this.id = id;
        this.name = routeName;
        this.abonent = null;
        this.routeSite = null;
        this.service = null;
        this.enabled = false;
        this.defaultRoute = false;
        this.transit = false;
        this.saa = true;
    }

    public HttpRoute(final Long id, final Element routeElement, final Map subjects, final ServiceProvidersManager serviceProvidersManager)
            throws SibincoException {
        this.id = id;
        this.name = routeElement.getAttribute("name");
        if (name.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new SibincoException("Http route name is too long: " + name.length() + " chars \"" + name + '"');

        this.enabled = routeElement.getAttribute("enabled").equalsIgnoreCase("true");
        this.defaultRoute = routeElement.getAttribute("default").equalsIgnoreCase("true");

        String trStr = routeElement.getAttribute("transit");
        this.transit = routeElement.getAttribute("transit").equalsIgnoreCase("true");

        String saaStr = routeElement.getAttribute("saa"); 
        if( !trStr.equals("") && saaStr.equals("") ){
            if( trStr.equalsIgnoreCase("true") ){
                this.saa = false;
            } else {
                this.saa = true;
            }
        } else {
            this.saa = saaStr.equalsIgnoreCase("true");
        }
        final Long serviceId = Long.decode(routeElement.getAttribute("serviceId"));
        this.service = serviceProvidersManager.getServiceById(serviceId);
        abonent = loadAbonent(routeElement, subjects);
        routeSite = loadRouteSite(routeElement);
    }


    private Abonent loadAbonent(Element routeElement, Map subjects) throws SibincoException {
        return new Abonent(routeElement, subjects);
    }

    private RouteSite loadRouteSite(Element routeElement) throws SibincoException {
        return new RouteSite(routeElement);
    }

    public String getName() {
        if(name != null)name.trim();
        return name;
    }

    public PrintWriter store(PrintWriter out) {
        String name = StringEncoderDecoder.encode(getName());
        try {
            out.println("  <route id=\"" + id +
                    "\" serviceId=\"" + service.getId() +
                    "\" enabled=\"" + isEnabled() +
                    "\" default=\""+ isDefaultRoute() +
                    "\" name=\"" + name +
                    "\" transit=\"" + isTransit() +
                    "\" saa=\"" + isSaa() +
                    "\">");
            abonent.store(out);
            routeSite.store(out);
            out.println("  </route>");
            return out;
        } catch (Exception e) {
            logger.error("Couldn't save route " + name, e);
        }
        return null;
    }

    public void setName(final String name) {
        if (name.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Http rote name is too long");
        this.name = name;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Service getService() {
        return service;
    }

    public void setService(final Service service) {
        this.service = service;
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

    public Abonent getAbonent() {
        return abonent;
    }

    public void setAbonent(final Abonent abonent) {
        this.abonent = abonent;
    }

    public RouteSite getRouteSite() {
        return routeSite;
    }

    public void setRouteSite(final RouteSite routeSite) {
        this.routeSite = routeSite;
    }

    public boolean isSaa() {
        return saa;
    }

    public void setSaa(boolean saa) {
        this.saa = saa;
    }
}
