/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;

/**
 * The <code>Route</code> class represents
 */
public class Route {

    private final Logger logger = Logger.getLogger(this.getClass());

    private String name;
    private Map<String, Source> sources;
    private Map<String, Destination> destinations;
    private boolean enabled = true;
    private boolean transit = true;
    private boolean saa = false;
    private boolean hideSaaText = false;
    private String slicing;
    private String slicedRespPolicy;
    private String srcSmeId;
    private Service service;
    private String notes;

    public Route(final String routeName, final Map<String, Source> sources, final Map<String, Destination> destinations,
                 final boolean enabled, final String slicing, final String slicedRespPolicy,
                 final String srcSmeId, final Service service, final String notes) {
        if (routeName == null)
            throw new NullPointerException("Route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");
        if (sources == null)
            throw new NullPointerException("Sources list is null");
        if (destinations == null)
            throw new NullPointerException("Destinations list is null");

        this.name = routeName;
        this.sources = sources;
        this.destinations = destinations;        
        this.enabled = enabled;
        this.slicing = slicing;
        this.slicedRespPolicy = slicedRespPolicy;
        this.srcSmeId = srcSmeId;
        this.service = service;
        this.notes = notes;
    }

    public Route(final String routeName, final Map<String, Source> sources, final Map<String, Destination> destinations,
                 final boolean enabled, final String slicing, final String slicedRespPolicy,
                 final String srcSmeId, final Service service, final String notes, final boolean transit) {
        if (routeName == null)
            throw new NullPointerException("Route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");
        if (sources == null)
            throw new NullPointerException("Sources list is null");
        if (destinations == null)
            throw new NullPointerException("Destinations list is null");

        this.name = routeName;
        this.sources = sources;
        this.destinations = destinations;
        this.enabled = enabled;
        this.transit = transit;
        this.slicing = slicing;
        this.slicedRespPolicy = slicedRespPolicy;
        this.srcSmeId = srcSmeId;
        this.service = service;
        this.notes = notes;
    }

    public Route( final String routeName, final Map<String, Source> sources, final Map<String, Destination> destinations,
                 final boolean enabled, final String slicing, final String slicedRespPolicy,
                 final String srcSmeId, final Service service, final String notes, final boolean transit,
                 boolean saa, boolean hideSaaText) {
        if (routeName == null)
            throw new NullPointerException("Route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");
        if (sources == null)
            throw new NullPointerException("Sources list is null");
        if (destinations == null)
            throw new NullPointerException("Destinations list is null");

        this.name = routeName;
        this.sources = sources;
        this.destinations = destinations;
        this.enabled = enabled;
        this.transit = transit;
        this.saa = saa;
        this.hideSaaText = hideSaaText;
        this.slicing = slicing;
        this.slicedRespPolicy = slicedRespPolicy;
        this.srcSmeId = srcSmeId;
        this.service = service;
        this.notes = notes;
    }

    public Route(String routeName) {
        if (routeName == null)
            throw new NullPointerException("Route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");

        this.name = routeName;
        this.sources = new HashMap<String, Source>();
        this.destinations = new HashMap<String, Destination>();
        this.enabled = false;
        this.transit = false;
        this.slicing = "NONE";
        this.slicedRespPolicy = "ALL";
        this.srcSmeId = "";
        this.service = null;
        this.notes = "";
    }


    public Route(Element routeElem, Map subjects, SmppManager smppManager,
                 ServiceProvidersManager serviceProvidersManager) throws SibincoException{
        name = routeElem.getAttribute("id");
        if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
            throw new SibincoException("Route name is too long: " + name.length() + " chars \"" + name + '"');
        }
        sources = loadSources(routeElem, subjects);
        destinations = loadDestinations(routeElem, subjects, smppManager);
        enabled = routeElem.getAttribute("enabled").equalsIgnoreCase("true");

        String trStr = routeElem.getAttribute("transit");
        transit = routeElem.getAttribute("transit").equalsIgnoreCase("true");

        String saaStr = routeElem.getAttribute("saa");
        if( !trStr.equals("") && saaStr.equals("") ){
            saa = !trStr.equals("true");
        } else {
            saa = routeElem.getAttribute("saa").equalsIgnoreCase("true");
        }
        
        hideSaaText = routeElem.getAttribute("hideSaaText").equalsIgnoreCase("true");

        slicing = routeElem.getAttribute("slicing");
        slicedRespPolicy = routeElem.getAttribute("slicedRespPolicy");
        srcSmeId = routeElem.getAttribute("srcSmeId");
        final Long serviceId = Long.decode(routeElem.getAttribute("serviceId"));
        this.service = serviceProvidersManager.getServiceById(serviceId);
        notes = "";
        NodeList notesList = routeElem.getElementsByTagName("notes");
        for (int i = 0; i < notesList.getLength(); i++)
            notes += Utils.getNodeText(notesList.item(i));
    }

    private Map<String, Source> loadSources(Element routElement, Map subjects) {
        Map<String, Source> result = new HashMap<String, Source>();
        NodeList list = routElement.getElementsByTagName("source");
        for (int i = 0; i < list.getLength(); i++) {
            try {
                final Source source = new Source((Element) list.item(i), subjects);
                result.put(source.getName(), source);
            } catch (SibincoException e) {
                logger.warn("Couldn't load source, skipped", e);
            }
        }
        return result;
    }

    private Map<String, Destination> loadDestinations(Element routeElem, Map subjects, SmppManager smppManager){
        Map<String, Destination> result = new HashMap<String, Destination>();
        NodeList list = routeElem.getElementsByTagName("destination");
        for (int i = 0; i < list.getLength(); i++) {
            Destination destination;
            try {
                destination = new Destination((Element) list.item(i), subjects, smppManager);
                result.put(destination.getName(), destination);
            } catch (SibincoException e) {
                logger.warn("Could not create destination with id=" + list.item(i).getLocalName() + "child=" + list.item(i).getFirstChild().getLocalName() );
            }
        }
        return result;
    }

    public PrintWriter  store(PrintWriter out) {
        String name = StringEncoderDecoder.encode(getName());
        try {
            out.println("  <route id=\"" + name
                    + "\" enabled=\"" + isEnabled()
                    + "\" transit=\"" + isTransit()
                    + "\" saa=\"" + isSaa()
                    + "\" hideSaaText=\"" + isHideSaaText()
                    + "\" slicing=\"" + StringEncoderDecoder.encode(getSlicing())
                    + "\" slicedRespPolicy=\"" + StringEncoderDecoder.encode(getSlicedRespPolicy())
                    + "\" srcSmeId=\"" + StringEncoderDecoder.encode(getSrcSmeId())
                    + "\" serviceId=\"" + getService().getId()
                    + "\">");
            if (notes != null)
                out.println("    <notes>" + notes + "</notes>");
            for (Source source : sources.values()) source.store(out);
            for (Destination destination : destinations.values()) destination.store(out);
            out.println("  </route>");
            return out;
        } catch (Exception e) {
            logger.error("Couldn't save route " + name, e);
        }
        return null;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        if (name.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");
        this.name = name;
    }

    public Map<String, Source> getSources() {
        return sources;
    }

    public Map<String, Destination> getDestinations() {
        return destinations;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isTransit() {
        return transit;
    }

    public void setTransit(final boolean transit) {
        this.transit = transit;
    }

    public boolean isSaa() {
        return saa;
    }

    public void setSaa(boolean saa) {
        this.saa = saa;
    }

    public boolean isHideSaaText() {
        return hideSaaText;
    }

    public String getSlicing() {
        return slicing;
    }

    public String getSlicedRespPolicy() {
        return slicedRespPolicy;
    }

    public String getSrcSmeId() {
        return srcSmeId;
    }

    public Service getService() {
        return service;
    }

    public void setService(Service service) {
        this.service = service;
    }

    public String getServiceName() {
        if (null != service)
            return service.getName();
        else
            return null;
    }

    public String getNotes() {
        return notes;
    }

    public void setNotes(final String notes) {
        this.notes = notes;
    }

}
