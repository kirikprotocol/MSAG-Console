/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:43:03
 */
package ru.sibinco.lib.backend.route;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.xml.Utils;

import java.io.PrintWriter;
import java.util.*;


public class Route
{
  private final Logger logger = Logger.getLogger(this.getClass());

  private String name = null;
  private Map src = null;
  private Map dst = null;
  private int priority = 0;
  private boolean enabling = true;
  private boolean archiving = true;
  private boolean billing = false;
  private int serviceId = 0;
  private boolean suppressDeliveryReports = false;
  private boolean active;
  private String srcSmeId;
  private String deliveryMode = null;
  private String forwardTo = null;
  private boolean hide = false;
  private boolean forceReplayPath = false;
  private String notes = "";

  public Route(String routeName, int priority, boolean isEnabling, boolean isBilling, boolean isArchiving, boolean isSuppressDeliveryReports, boolean active, int serviceId, Map sources, Map destinations, String srcSmeId, String deliveryMode, String forwardTo, boolean hide, boolean forceReplayPath, String notes)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");
    if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");
    if (sources == null)
      throw new NullPointerException("Sources list is null");
    if (destinations == null)
      throw new NullPointerException("Destinations list is null");

    this.name = routeName;
    this.priority = priority;
    this.src = sources;
    this.dst = destinations;
    this.enabling = isEnabling;
    this.archiving = isArchiving;
    this.billing = isBilling;
    this.serviceId = serviceId;
    this.suppressDeliveryReports = isSuppressDeliveryReports;
    this.active = active;
    this.srcSmeId = srcSmeId;
    this.deliveryMode = deliveryMode;
    this.forwardTo = forwardTo;
    this.hide = hide;
    this.forceReplayPath = forceReplayPath;
    this.notes = notes;
  }

  public Route(String routeName)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");
    if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");

    name = routeName;
    src = new HashMap();
    dst = new HashMap();
    priority = 0;
    enabling = false;
    archiving = false;
    billing = false;
    serviceId = 0;
    suppressDeliveryReports = false;
    active = false;
    srcSmeId = "";
    deliveryMode = "default";
    forwardTo = "";
    hide = true;
    forceReplayPath = false;
    notes = "";
  }

  public Route(Element routeElem, Map subjects, SmeManager smeManager) throws SibincoException
  {
    name = routeElem.getAttribute("id");
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
      throw new SibincoException("Route name is too long: " + name.length() + " chars \"" + name + '"');
    }
    src = loadSources(routeElem, subjects);
    dst = loadDestinations(routeElem, subjects, smeManager);
    priority = Integer.decode(routeElem.getAttribute("priority")).intValue();
    enabling = routeElem.getAttribute("enabling").equalsIgnoreCase("true");
    archiving = routeElem.getAttribute("archiving").equalsIgnoreCase("true");
    billing = routeElem.getAttribute("billing").equalsIgnoreCase("true");
    serviceId = Integer.decode(routeElem.getAttribute("serviceId")).intValue();
    suppressDeliveryReports = Boolean.valueOf(routeElem.getAttribute("suppressDeliveryReports")).booleanValue();
    active = Boolean.valueOf(routeElem.getAttribute("active")).booleanValue();
    srcSmeId = routeElem.getAttribute("srcSmeId");
    deliveryMode = routeElem.getAttribute("deliveryMode");
    forwardTo = routeElem.getAttribute("forwardTo");
    hide = routeElem.getAttribute("hide").length() > 0 ? routeElem.getAttribute("hide").equalsIgnoreCase("true") : true;
    forceReplayPath = routeElem.getAttribute("forceReplayPath").equalsIgnoreCase("true");
    notes = "";
    NodeList notesList = routeElem.getElementsByTagName("notes");
    for (int i = 0; i < notesList.getLength(); i++)
      notes += Utils.getNodeText(notesList.item(i));
  }

  private Map loadSources(Element routeElem, Map subjects)
  {
    Map result = new HashMap();
    NodeList list = routeElem.getElementsByTagName("source");
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

  private Map loadDestinations(Element routeElem, Map subjects, SmeManager smeManager)
      throws SibincoException
  {
    Map result = new HashMap();
    NodeList list = routeElem.getElementsByTagName("destination");
    for (int i = 0; i < list.getLength(); i++) {
      final Destination destination = new Destination((Element) list.item(i), subjects, smeManager);
      result.put(destination.getName(), destination);
    }
    return result;
  }

  public String getId()
  {
    return getName();
  }

  public void setId(String id)
  {
    setName(id);
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");
    this.name = name;
  }

  public void addSource(Source newSrc)
  {
    if (newSrc == null)
      throw new NullPointerException("Source is null");

    src.put(newSrc.getName(), newSrc);
  }

  public Source removeSource(String sourceName)
  {
    return (Source) src.remove(sourceName);
  }

  public void addDestination(Destination newDst)
  {
    if (newDst == null)
      throw new NullPointerException("Destination is null");

    dst.put(newDst.getName(), newDst);
  }

  public Destination removeDestination(String destinationName)
  {
    return (Destination) dst.remove(destinationName);
  }

  public Map getSources()
  {
    return src;
  }

  public Map getDestinations()
  {
    return dst;
  }

  public void updateSources(Set sourcesStrings, String masksString, Map allSubjects)
  {
    Map source_selected = new HashMap();
    for (Iterator i = sourcesStrings.iterator(); i.hasNext();) {
      String token = (String) i.next();
      final Source source = new Source((Subject) allSubjects.get(token));
      source_selected.put(source.getName(), source);
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      Mask m = (Mask) i.next();
      final Source source = new Source(m);
      source_selected.put(source.getName(), source);
    }

    src.keySet().retainAll(source_selected.keySet());
    source_selected.keySet().removeAll(src.keySet());
    src.putAll(source_selected);
  }

  public void updateDestinations(Set destinationsStrings, String masksString, Map allSubjects, Sme defaultSme) throws SibincoException
  {
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

    dst.keySet().retainAll(list.keySet());
    list.keySet().removeAll(dst.keySet());
    dst.putAll(list);
  }

  public boolean isEnabling()
  {
    return enabling;
  }

  public void setEnabling(boolean enabling)
  {
    this.enabling = enabling;
  }

  public boolean isBilling()
  {
    return billing;
  }

  public void setBilling(boolean billing)
  {
    this.billing = billing;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(boolean archiving)
  {
    this.archiving = archiving;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("  <route id=\"" + StringEncoderDecoder.encode(getName()) + "\" billing=\"" + isBilling()
                + "\" archiving=\"" + isArchiving() + "\" enabling=\"" + isEnabling() + "\" priority=\"" + getPriority()
                + "\" serviceId=\"" + getServiceId() + "\" suppressDeliveryReports=\"" + isSuppressDeliveryReports()
                + "\" active=\"" + isActive() + "\" srcSmeId=\"" + StringEncoderDecoder.encode(getSrcSmeId())
                + "\" deliveryMode=\"" + StringEncoderDecoder.encode(getDeliveryMode())
                + "\" hide=\"" + (isHide() ? "true" : "false")
                + "\" forceReplayPath=\"" + (isForceReplayPath() ? "true" : "false")
                + ("MAP_PROXY".equals(getSrcSmeId()) ? "\" forwardTo=\"" + StringEncoderDecoder.encode(getForwardTo()) : "")
                + "\">");
    if (notes != null)
      out.println("    <notes>" + notes + "</notes>");

    for (Iterator i = src.values().iterator(); i.hasNext();) {
      Source source = (Source) i.next();
      source.store(out);
    }
    for (Iterator i = dst.values().iterator(); i.hasNext();) {
      Destination destination = (Destination) i.next();
      destination.store(out);
    }
    out.println("  </route>");
    return out;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(int priority)
  {
    this.priority = priority;
  }

  public int getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(int serviceId)
  {
    this.serviceId = serviceId;
  }

  public boolean isSuppressDeliveryReports()
  {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getDeliveryMode()
  {
    return deliveryMode;
  }

  public void setDeliveryMode(String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(boolean hide)
  {
    this.hide = hide;
  }

  public boolean isForceReplayPath()
  {
    return forceReplayPath;
  }

  public void setForceReplayPath(boolean forceReplayPath)
  {
    this.forceReplayPath = forceReplayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(String notes)
  {
    this.notes = notes;
  }
}
