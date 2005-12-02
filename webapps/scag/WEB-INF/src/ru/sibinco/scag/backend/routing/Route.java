/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.route.MaskList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.sme.Category;
import ru.sibinco.scag.backend.sme.CategoryManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.rules.RuleManager;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * The <code>Route</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 11:25:51
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Route {

    private final Logger logger = Logger.getLogger(this.getClass());

    private String name;
    private Map sources;
    private Map destinations;
    private boolean archived;
    private boolean enabled;
    private boolean active;
    private String srcSmeId;
    private Provider provider;
    private Category category;
    private Rule rule;
    private String notes;

    public Route(final String routeName, final Map sources, final Map destinations, final boolean archived,
                 final boolean enabled, final boolean active, final String srcSmeId, final Provider provider,
                 final Category category, final Rule rule, final String notes) {
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
        this.archived = archived;
        this.enabled = enabled;
        this.active = active;
        this.srcSmeId = srcSmeId;
        this.provider = provider;
        this.category = category;
        this.rule = rule;
        this.notes = notes;
    }

    public Route(String routeName) {
        if (routeName == null)
            throw new NullPointerException("Route name is null");
        if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
            throw new IllegalArgumentException("Route name is too long");

        this.name = routeName;
        this.sources = new HashMap();
        this.destinations = new HashMap();
        this.archived = false;
        this.enabled = false;
        this.active = false;
        this.srcSmeId = "";
        this.provider = null;
        this.category = null;
        this.rule = null;
        this.notes = "";
    }


    public Route(Element routeElem, Map subjects, SmppManager smppManager,
                 RuleManager ruleManager, ProviderManager providerManager,
                 CategoryManager categoryManager) throws SibincoException {

        name = StringEncoderDecoder.encode(routeElem.getAttribute("id"));
        if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
            throw new SibincoException("Route name is too long: " + name.length() + " chars \"" + name + '"');
        }
        sources = loadSources(routeElem, subjects);
        destinations = loadDestinations(routeElem, subjects, smppManager);
        archived = routeElem.getAttribute("archived").equalsIgnoreCase("true");
        enabled = routeElem.getAttribute("enabled").equalsIgnoreCase("true");
        active = routeElem.getAttribute("active").equalsIgnoreCase("true");
        srcSmeId = routeElem.getAttribute("srcSmeId");
        final Long providerId = Long.decode(routeElem.getAttribute("providerId"));
        this.provider = (Provider) providerManager.getProviders().get(providerId);
        final Long categoryId = Long.decode(routeElem.getAttribute("categoryId"));
        this.category = (Category) categoryManager.getCategories().get(categoryId);
        final Long ruleId = Long.decode(routeElem.getAttribute("ruleId"));
        this.rule = (Rule) ruleManager.getRules().get(ruleId);
        notes = "";
        NodeList notesList = routeElem.getElementsByTagName("notes");
        for (int i = 0; i < notesList.getLength(); i++)
            notes += Utils.getNodeText(notesList.item(i));
    }

    private Map loadSources(Element routElement, Map subjects) {
        Map result = new HashMap();
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

    private Map loadDestinations(Element routeElem, Map subjects, SmppManager smppManager)
            throws SibincoException {
        Map result = new HashMap();
        NodeList list = routeElem.getElementsByTagName("destination");
        for (int i = 0; i < list.getLength(); i++) {
            final Destination destination = new Destination((Element) list.item(i), subjects, smppManager);
            result.put(destination.getName(), destination);
        }
        return result;
    }

    public void updateSources(Set sourcesString, String masksString, Map allSubjects) {
        Map sources_selected = new HashMap();
        for (Iterator iterator = sourcesString.iterator(); iterator.hasNext();) {
            String token = (String) iterator.next();
            final Source source = new Source((Subject) allSubjects.get(token));
            sources_selected.put(source.getName(), source);
        }
        MaskList masks = new MaskList(masksString);
        for (Iterator it = masks.iterator(); it.hasNext();) {
            Mask mask = (Mask) it.next();
            final Source source = new Source(mask);
            sources_selected.put(source.getName(), source);
        }
        sources.keySet().retainAll(sources_selected.keySet());
        sources_selected.keySet().removeAll(sources.keySet());
        sources.putAll(sources_selected);
    }

    public void updateDestinations(Set destinationsStrings, String masksString, Map allSubjects, Svc defaultSme) throws SibincoException {
        Map list = new HashMap();
        for (Iterator i = destinationsStrings.iterator(); i.hasNext();) {
            String token = (String) i.next();
            final Destination destination = new Destination((Subject) allSubjects.get(token));
            list.put(destination.getName(), destination);
        }

        MaskList masks = new MaskList(masksString);
        for (Iterator i = masks.iterator(); i.hasNext();) {
            Mask m = (Mask) i.next();
            final Destination destination = new Destination(m, defaultSme);
            list.put(destination.getName(), destination);
        }
        destinations.keySet().retainAll(list.keySet());
        list.keySet().removeAll(destinations.keySet());
        destinations.putAll(list);
    }

    public PrintWriter store(PrintWriter out) {
        String name = StringEncoderDecoder.encode(getName());
        try {
            out.println("  <route id=\"" + name
                    + "\" archived=\"" + isArchived()
                    + "\" enabled=\"" + isEnabled()
                    + "\" active=\"" + isActive()
                    + "\" srcSmeId=\"" + StringEncoderDecoder.encode(getSrcSmeId())
                    + "\" providerId=\"" + getProvider().getId()
                    + "\" categoryId=\"" + getCategory().getId()
                    + "\" ruleId=\"" + getRule().getId()
                    + "\">");
            if (notes != null)
                out.println("    <notes>" + notes + "</notes>");

            for (Iterator i = sources.values().iterator(); i.hasNext();) {
                Source source = (Source) i.next();
                source.store(out);
            }
            for (Iterator i = destinations.values().iterator(); i.hasNext();) {
                Destination destination = (Destination) i.next();
                destination.store(out);
            }
            out.println("  </route>");
            return out;
        } catch (Exception e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
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

    public void addSources(Source newSource) {
        if (newSource == null)
            throw new NullPointerException("Source is null");
        sources.put(newSource.getName(), newSource);
    }

    public Source removeSource(String sourceName) {
        return (Source) sources.remove(sourceName);
    }

    public void addDestination(Destination newDestination) {
        if (newDestination == null)
            throw new NullPointerException("Destination is null");

        destinations.put(newDestination.getName(), newDestination);
    }

    public Destination removeDestination(String destinationName) {
        return (Destination) destinations.remove(destinationName);
    }

    public Map getSources() {
        return sources;
    }

    public Map getDestinations() {
        return destinations;
    }

    public boolean isArchived() {
        return archived;
    }

    public void setArchived(final boolean archived) {
        this.archived = archived;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isActive() {
        return active;
    }

    public void setActive(final boolean active) {
        this.active = active;
    }

    public String getSrcSmeId() {
        return srcSmeId;
    }

    public void setSrcSmeId(final String srcSmeId) {
        this.srcSmeId = srcSmeId;
    }

    public Provider getProvider() {
        return provider;
    }

    public void setProvider(final Provider provider) {
        this.provider = provider;
    }

    public String getProviderName() {
        if (null != provider)
            return provider.getName();
        else
            return null;
    }

    public Category getCategory() {
        return category;
    }

    public void setCategory(final Category category) {
        this.category = category;
    }

    public String getCategoryName() {
        if (null != category)
            return category.getName();
        else
            return null;
    }

    public Rule getRule() {
        return rule;
    }

    public void setRule(final Rule rule) {
        this.rule = rule;
    }

    public String getRuleName() {
        if (null != rule)
            return rule.getName();
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
