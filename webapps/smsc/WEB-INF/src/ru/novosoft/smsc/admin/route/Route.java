/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:43:03
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.PrintWriter;
import java.util.Iterator;
import java.util.Set;


public class Route
{
  private String name = null;
  private SourceList src = null;
  private DestinationList dst = null;
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
  private boolean forceDelivery = false;
  private long aclId;

  public Route(String routeName, int priority, boolean isEnabling, boolean isBilling, boolean isArchiving, boolean isSuppressDeliveryReports, boolean active, int serviceId, SourceList sources, DestinationList destinations, String srcSmeId, String deliveryMode, String forwardTo, boolean hide, boolean forceReplayPath, String notes, boolean forceDelivery, long aclId)
  {
    this.aclId = aclId;
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
    this.forceDelivery = forceDelivery;
  }

  public Route(String routeName)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");
    if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");

    name = routeName;
    src = new SourceList();
    dst = new DestinationList();
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
    forceDelivery = false;
    aclId = -1;
  }

  public Route(Element routeElem, SubjectList subjects, SmeManager smeManager) throws AdminException
  {
    name = routeElem.getAttribute("id");
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
      throw new AdminException("Route name is too long: " + name.length() + " chars \"" + name + '"');
    }
    src = new SourceList(routeElem, subjects);
    dst = new DestinationList(routeElem, subjects, smeManager);
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
    forceDelivery = Boolean.valueOf(routeElem.getAttribute("forceDelivery")).booleanValue();
    aclId = Long.decode(routeElem.getAttribute("aclId")).longValue();
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

    src.add(newSrc);
  }

  public Source removeSource(String sourceName)
  {
    return src.remove(sourceName);
  }

  public Destination removeDestination(String destinationName)
  {
    return dst.remove(destinationName);
  }

  public void addDestination(Destination newDst)
  {
    if (newDst == null)
      throw new NullPointerException("Destination is null");

    dst.add(newDst);
  }

  public SourceList getSources()
  {
    return src;
  }

  public DestinationList getDestinations()
  {
    return dst;
  }

  public void updateSources(Set sourcesStrings, String masksString, SubjectList allSubjects)
  {
    SourceList source_selected = new SourceList();
    for (Iterator i = sourcesStrings.iterator(); i.hasNext();) {
      String token = (String) i.next();
      source_selected.add(new Source(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      Mask m = (Mask) i.next();
      source_selected.add(new Source(m));
    }

    src.retainAll(source_selected);
    source_selected.removeAll(src);
    src.addAll(source_selected);
  }

  public void updateDestinations(Set destinationsStrings, String masksString, SubjectList allSubjects, SME defaultSme) throws AdminException
  {
    DestinationList list = new DestinationList();
    for (Iterator i = destinationsStrings.iterator(); i.hasNext();) {
      String token = (String) i.next();
      list.add(new Destination(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      Mask m = (Mask) i.next();
      list.add(new Destination(m, defaultSme));
    }

    dst.retainAll(list);
    list.removeAll(dst);
    dst.addAll(list);
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
                + "\" aclId=\"" + getAclId()
                + "\">");
    if (notes != null)
      out.println("    <notes>" + notes + "</notes>");
    getSources().store(out);
    getDestinations().store(out);
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

  public boolean isForceDelivery()
  {
    return forceDelivery;
  }

  public void setForceDelivery(boolean forceDelivery)
  {
    this.forceDelivery = forceDelivery;
  }

  public long getAclId()
  {
    return aclId;
  }

  public void setAclId(long aclId)
  {
    this.aclId = aclId;
  }
}
