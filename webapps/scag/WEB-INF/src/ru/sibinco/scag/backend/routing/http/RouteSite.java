/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.*;
import java.io.PrintWriter;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.routing.http.placement.SitePlacement;

/**
 * The <code>RouteSite</code> class represents
 * <p><p/>
 * <p/>
 * <p/>
 * Date: 16.06.2006
 * Time: 15:49:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class RouteSite {

    private Map siteSubjects = new HashMap();
    private Map sites = new HashMap();
    private List usrPlace = new ArrayList();
    private List addressPlace = new ArrayList();


    public RouteSite(Map siteSubjects, Map sites) {
        this.siteSubjects = siteSubjects;
        this.sites = sites;
    }

    public RouteSite() {
    }

    public RouteSite(Element routeElement) throws SibincoException {
        NodeList subjSiteList = routeElement.getElementsByTagName("site_subject");
        for (int i = 0; i < subjSiteList.getLength(); i++) {
            Element subjSiteElement = (Element) subjSiteList.item(i);
            final HttpSite httpSite = new HttpSite(subjSiteElement.getAttribute("id"));
            siteSubjects.put(httpSite.getName(), httpSite);
        }
        NodeList listSite = routeElement.getElementsByTagName("site");
        for (int i = 0; i < listSite.getLength(); i++) {
            Element siteElement = (Element) listSite.item(i);
            final Site site = new Site(siteElement.getAttribute("host"), Integer.parseInt(siteElement.getAttribute("port")));
            NodeList pathNodeList = siteElement.getElementsByTagName("path");
            List list = new ArrayList();
            for (int j = 0; j < pathNodeList.getLength(); j++) {
                Element pathElement = (Element) pathNodeList.item(j);
                list.add(pathElement.getAttribute("value"));
            }
            site.setPathLinks((String[]) list.toArray(new String[list.size()]));
            sites.put(site.getHost(), site);
        }
        NodeList placementList = routeElement.getElementsByTagName("placement");
        if (placementList != null) {
            Element usrplacementElement = (Element) placementList.item(0);
            Element addrplacementElement = (Element) placementList.item(1);
            if (usrplacementElement != null) {
                NodeList usrPlaceNodeList = usrplacementElement.getElementsByTagName("usr_place");
                if (usrPlaceNodeList != null) {
                    for (int i = 0; i < usrPlaceNodeList.getLength(); i++) {
                        Element usrPlaceElement = (Element) usrPlaceNodeList.item(i);
                        final SitePlacement sitePlacement = new SitePlacement(
                                usrPlaceElement.getAttribute("type"),
                                usrPlaceElement.getAttribute("name"));
                        usrPlace.add(sitePlacement);
                    }
                }
            }
            if (addrplacementElement != null) {
                NodeList addressPlaceNodeList = addrplacementElement.getElementsByTagName("address_place");
                if (addressPlaceNodeList != null) {
                    for (int i = 0; i < addressPlaceNodeList.getLength(); i++) {
                        Element addressPlaceElement = (Element) addressPlaceNodeList.item(i);
                        final SitePlacement sitePlacement = new SitePlacement(
                                addressPlaceElement.getAttribute("type"),
                                addressPlaceElement.getAttribute("name"));
                        addressPlace.add(sitePlacement);
                    }
                }
            }
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <sites>");
        for (Iterator i = siteSubjects.values().iterator(); i.hasNext();) {
            final HttpSite httpSite = (HttpSite) i.next();
            out.println("        <site_subject id=\"" + StringEncoderDecoder.encode(httpSite.getName()) + "\"/>");
        }
        for (Iterator i = sites.values().iterator(); i.hasNext();) {
            final Site site = (Site) i.next();
            out.println("        <site host=\"" + StringEncoderDecoder.encode(site.getHost()) + "\"" +
                    " port=\"" + site.getPort() + "\">");
            for (int j = 0; j < site.getPathLinks().length; j++) {
                out.println("            <path value=\"" + StringEncoderDecoder.encode(site.getPathLinks()[j]) + "\"/>");
            }
            out.println("        </site>");
        }
        out.println();
        out.println("        <placement>");
        for (Iterator i = addressPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println(new StringBuffer().append("            <address_place type=\"").
                    append(StringEncoderDecoder.encode(placement.getType())).append("\" ").
                    append(" name=\"").append(StringEncoderDecoder.encode(placement.getName())).
                    append("\" />").toString());
        }
        out.println();
        for (Iterator i = usrPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println(new StringBuffer().append("            <usr_place type=\"").
                    append(StringEncoderDecoder.encode(placement.getType())).
                    append("\" ").append(" name=\"").append(StringEncoderDecoder.encode(placement.getName())).
                    append("\" />").toString());
        }
        out.println("        </placement>");
        out.println();
        out.println("    </sites>");
        out.println();
        return out;
    }

    public Map getSiteSubjects() {
        return siteSubjects;
    }

    public Map getSites() {
        return sites;
    }

    public String[] getSiteSubjAsString() {
        List subjSiteList = new ArrayList();
        for (Iterator i = siteSubjects.values().iterator(); i.hasNext();) {
            final HttpSite httpSite = (HttpSite) i.next();
            subjSiteList.add(httpSite.getName());
        }
        return (String[]) subjSiteList.toArray(new String[subjSiteList.size()]);
    }

    public Site[] getArraySite() {
        List siteList = new ArrayList();
        for (Iterator i = sites.values().iterator(); i.hasNext();) {
            final Site site = (Site) i.next();
            siteList.add(site);
        }
        return (Site[]) siteList.toArray(new Site[siteList.size()]);
    }

    public String[] getSiteAsStr() {
        List siteList = new ArrayList();
        for (Iterator i = sites.values().iterator(); i.hasNext();) {
            final Site site = (Site) i.next();
            siteList.add(site.getHost());
        }
        return (String[]) siteList.toArray(new String[siteList.size()]);
    }

    public List getUsrPlace() {
        return usrPlace;
    }

    public List getAddressPlace() {
        return addressPlace;
    }
}
