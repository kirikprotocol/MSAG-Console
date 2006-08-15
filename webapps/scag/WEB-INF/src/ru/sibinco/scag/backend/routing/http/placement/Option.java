/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http.placement;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.util.*;
import java.io.PrintWriter;

/**
 * The <code>Option</code> class represents
 * <p><p/>
 * <p/>
 * Date: 04.07.2006
 * Time: 17:33:41
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Option {

    private List abonAddressPlace = new ArrayList();
    private List abonUsrPlace = new ArrayList();
    private List abonRouteIdPlace = new ArrayList();
    private List abonServiceIdPlace = new ArrayList();
    private List siteAddressPlace = new ArrayList();
    private List siteUsrPlace = new ArrayList();
    private List siteRouteIdPlace = new ArrayList();
    private List siteServiceIdPlace = new ArrayList();

    public Option(final NodeList optionsList) throws SibincoException {

        Element optionsElement = (Element) optionsList.item(0);
        if (optionsElement != null) {
            NodeList abonentList = optionsElement.getElementsByTagName("abonents");
            NodeList sitesList = optionsElement.getElementsByTagName("sites");
            Element abonentElement = (Element) abonentList.item(0);
            Element sitesElement = (Element) sitesList.item(0);

            NodeList addressPlaceList = abonentElement.getElementsByTagName("address_place");
            NodeList usrPlaceList = abonentElement.getElementsByTagName("usr_place");
            NodeList routeIdPlaceList = abonentElement.getElementsByTagName("route_id_place");
            NodeList serviceIdPlaceList = abonentElement.getElementsByTagName("service_id_place");
            for (int i = 0; i < addressPlaceList.getLength(); i++) {
                Element addressPlaceElement = (Element) addressPlaceList.item(i);
                final AbonentsPlacement abonentsPlacement = new AbonentsPlacement(
                        addressPlaceElement.getAttribute("type"),
                        addressPlaceElement.getAttribute("name"),
                        Integer.parseInt(addressPlaceElement.getAttribute("priority")));
                abonAddressPlace.add(abonentsPlacement);
            }
            for (int i = 0; i < usrPlaceList.getLength(); i++) {
                Element usrPlaceElement = (Element) usrPlaceList.item(i);
                final AbonentsPlacement abonentsPlacement = new AbonentsPlacement(
                        usrPlaceElement.getAttribute("type"),
                        usrPlaceElement.getAttribute("name"),
                        Integer.parseInt(usrPlaceElement.getAttribute("priority")));
                abonUsrPlace.add(abonentsPlacement);
            }
            for (int i = 0; i < routeIdPlaceList.getLength(); i++) {
                Element routeIdPlaceElement = (Element) routeIdPlaceList.item(i);
                final AbonentsPlacement abonentsPlacement = new AbonentsPlacement(
                        routeIdPlaceElement.getAttribute("type"),
                        routeIdPlaceElement.getAttribute("name"),
                        Integer.parseInt(routeIdPlaceElement.getAttribute("priority")));
                abonRouteIdPlace.add(abonentsPlacement);
            }
            for (int i = 0; i < serviceIdPlaceList.getLength(); i++) {
                Element serviceIdPlaceElement = (Element) serviceIdPlaceList.item(i);
                final AbonentsPlacement abonentsPlacement = new AbonentsPlacement(
                        serviceIdPlaceElement.getAttribute("type"),
                        serviceIdPlaceElement.getAttribute("name"),
                        Integer.parseInt(serviceIdPlaceElement.getAttribute("priority")));
                abonServiceIdPlace.add(abonentsPlacement);
            }

            NodeList sitesAddrPlaceList = sitesElement.getElementsByTagName("address_place");
            for (int i = 0; i < sitesAddrPlaceList.getLength(); i++) {
                Element sitesAddressPlaceElement = (Element) sitesAddrPlaceList.item(i);
                final SitePlacement sitePlacement = new SitePlacement(
                        sitesAddressPlaceElement.getAttribute("type"),
                        sitesAddressPlaceElement.getAttribute("name"));
                siteAddressPlace.add(sitePlacement);
            }
            NodeList sitesUsrPlaceList = sitesElement.getElementsByTagName("usr_place");
            for (int i = 0; i < sitesUsrPlaceList.getLength(); i++) {
                Element usrPlaceElement = (Element) sitesUsrPlaceList.item(i);
                final SitePlacement sitePlacement = new SitePlacement(
                        usrPlaceElement.getAttribute("type"),
                        usrPlaceElement.getAttribute("name"));
                siteUsrPlace.add(sitePlacement);
            }
            NodeList sitesRouteIdPlaceList = sitesElement.getElementsByTagName("route_id_place");
            for (int i = 0; i < sitesRouteIdPlaceList.getLength(); i++) {
                Element routeIdPlaceElement = (Element) sitesRouteIdPlaceList.item(i);
                final SitePlacement sitePlacement = new SitePlacement(
                        routeIdPlaceElement.getAttribute("type"),
                        routeIdPlaceElement.getAttribute("name"));
                siteRouteIdPlace.add(sitePlacement);
            }
            NodeList sitesServiceIdPlaceList = sitesElement.getElementsByTagName("service_id_place");
            for (int i = 0; i < sitesServiceIdPlaceList.getLength(); i++) {
                Element serviceIdPlaceElement = (Element) sitesServiceIdPlaceList.item(i);
                final SitePlacement sitePlacement = new SitePlacement(
                        serviceIdPlaceElement.getAttribute("type"),
                        serviceIdPlaceElement.getAttribute("name"));
                siteServiceIdPlace.add(sitePlacement);
            }
        }

    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <options>");
        out.println("        <abonents>");
        out.println("            <placement>");
        for (Iterator i = abonAddressPlace.iterator(); i.hasNext();) {
            final AbonentsPlacement placement = (AbonentsPlacement) i.next();
            out.println("                <address_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" "
                    + " priority=\"" + placement.getPriority() + "\" />");
        }
        out.println();
        for (Iterator i = abonUsrPlace.iterator(); i.hasNext();) {
            final AbonentsPlacement placement = (AbonentsPlacement) i.next();
            out.println("                <usr_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" "
                    + " priority=\"" + placement.getPriority() + "\" />");
        }
        out.println();
        for (Iterator i = abonRouteIdPlace.iterator(); i.hasNext();) {
            final AbonentsPlacement placement = (AbonentsPlacement) i.next();
            out.println("                <route_id_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" "
                    + " priority=\"" + placement.getPriority() + "\" />");
        }
        out.println();
        for (Iterator i = abonServiceIdPlace.iterator(); i.hasNext();) {
            final AbonentsPlacement placement = (AbonentsPlacement) i.next();
            out.println("                <service_id_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" "
                    + " priority=\"" + placement.getPriority() + "\" />");
        }
        out.println("            </placement>");
        out.println("        </abonents>");
        out.println();
        out.println("        <sites>");
        out.println("            <placement>");
        for (Iterator i = siteAddressPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println("                <address_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" />");
        }
        out.println();
        for (Iterator i = siteUsrPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println("                <usr_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" />");
        }
        out.println();
        for (Iterator i = siteRouteIdPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println("                <route_id_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" />");
        }
        out.println();
        for (Iterator i = siteServiceIdPlace.iterator(); i.hasNext();) {
            final SitePlacement placement = (SitePlacement) i.next();
            out.println("                <service_id_place type=\""
                    + StringEncoderDecoder.encode(placement.getType()) + "\" "
                    + " name=\"" + StringEncoderDecoder.encode(placement.getName()) + "\" />");
        }
        out.println("            </placement>");
        out.println("        </sites>");
        out.println("    </options>");
        return out;
    }

    public List getAbonAddressPlace() {
        return abonAddressPlace;
    }

    public List getAbonUsrPlace() {
        return abonUsrPlace;
    }

    public List getAbonRouteIdPlace() {
        return abonRouteIdPlace;
    }

    public List getAbonServiceIdPlace() {
        return abonServiceIdPlace;
    }

    public List getSiteAddressPlace() {
        return siteAddressPlace;
    }

    public List getSiteUsrPlace() {
        return siteUsrPlace;
    }

    public List getSiteRouteIdPlace() {
        return   siteRouteIdPlace;
    }

    public List getSiteServiceIdPlace() {
        return   siteServiceIdPlace;
    }

}
