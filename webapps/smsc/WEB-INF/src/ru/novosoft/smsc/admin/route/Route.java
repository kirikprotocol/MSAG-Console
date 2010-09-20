package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:43:03
 */

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
  public static final byte REPLAY_PATH_PASS = 0;
  public static final byte REPLAY_PATH_FORCE = 1;
  public static final byte REPLAY_PATH_SUPPRESS = 2;

  public static final byte BILLING_FALSE = 0;
  public static final byte BILLING_TRUE = 1;
  public static final byte BILLING_MT = 2;
  public static final byte BILLING_FR = 3;
  public static final byte BILLING_ON_SUBMIT = 4;
  public static final byte BILLING_CDR = 5;

  public static final int TRAFFIC_MODE_PROHIBITED = 0;
  public static final int TRAFFIC_MODE_SMS = 1;
  public static final int TRAFFIC_MODE_USSD = 2;
  public static final int TRAFFIC_MODE_ALL = 3;

  private String name = null;
  private SourceList src = null;
  private DestinationList dst = null;
  private int priority = 0;
//  private boolean enabling = true;
  private int trafficMode;
  private boolean archiving = true;
  private byte billing = 0;
  private boolean transit = false;
  private int serviceId = 0;
  private boolean suppressDeliveryReports = false;
  private boolean active;
  private String srcSmeId;
  private String deliveryMode = null;
  private String forwardTo = null;
  private boolean hide = false;
  private byte replayPath = REPLAY_PATH_PASS;
  private String notes = "";
  private boolean forceDelivery = false;
  private long aclId;
  private boolean allowBlocked;
  private long providerId;
  private long categoryId;
  private String backupSmeId;

  public Route(final String routeName, final int priority, final int trafficMode, final byte billing,final boolean isTransit, final boolean isArchiving,
               final boolean isSuppressDeliveryReports, final boolean active, final int serviceId, final SourceList sources,
               final DestinationList destinations, final String srcSmeId, final String deliveryMode, final String forwardTo, final boolean hide,
               final byte replayPath, final String notes, final boolean forceDelivery, final long aclId, final boolean allowBlocked, final long providerId, final long categoryId)
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
    this.trafficMode = trafficMode;
    this.archiving = isArchiving;
    this.billing = billing;
    this.transit = isTransit;
    this.serviceId = serviceId;
    this.suppressDeliveryReports = isSuppressDeliveryReports;
    this.active = active;
    this.srcSmeId = srcSmeId;
    this.deliveryMode = deliveryMode;
    this.forwardTo = forwardTo;
    this.hide = hide;
    this.replayPath = replayPath;
    this.notes = notes;
    this.forceDelivery = forceDelivery;
    this.allowBlocked = allowBlocked;
    this.aclId = aclId;
    this.providerId = providerId;
    this.categoryId = categoryId;
  }

  public Route(final String routeName)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");
    if (routeName.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");

    name = routeName;
    src = new SourceList();
    dst = new DestinationList();
    priority = 0;
    trafficMode = TRAFFIC_MODE_PROHIBITED;
//    enabling = false;
    archiving = false;
    billing = BILLING_FALSE;
    transit=false;
    serviceId = 0;
    suppressDeliveryReports = false;
    active = false;
    srcSmeId = "";
    deliveryMode = "default";
    forwardTo = "";
    hide = true;
    replayPath = REPLAY_PATH_PASS;
    notes = "";
    forceDelivery = false;
    aclId = -1;
    allowBlocked = false;
    providerId = -1;
    categoryId = -1;
  }


  public Route(final Element routeElem, final SubjectList subjects, final SmeManager smeManager) throws AdminException
  {
    name = routeElem.getAttribute("id");
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
      throw new AdminException("Route name is too long: " + name.length() + " chars \"" + name + '"');
    }
    src = new SourceList(routeElem, subjects);
    dst = new DestinationList(routeElem, subjects, smeManager);
    priority = Integer.decode(routeElem.getAttribute("priority")).intValue();
//    enabling = routeElem.getAttribute("enabling").equalsIgnoreCase("true");
    String enablingAttr = routeElem.getAttribute("enabling");
    if (enablingAttr != null) {
      trafficMode = enablingAttr.equals("true") ? TRAFFIC_MODE_ALL : TRAFFIC_MODE_PROHIBITED;
    } else {
      String trafficModeAttr = routeElem.getAttribute("trafficMode");
      if (trafficModeAttr != null) {
        if (trafficModeAttr.equals("none"))
          trafficMode = TRAFFIC_MODE_PROHIBITED;
        else if (trafficModeAttr.equals("smsOnly"))
          trafficMode = TRAFFIC_MODE_SMS;
        else if (trafficModeAttr.equals("ussdOnly"))
          trafficMode = TRAFFIC_MODE_USSD;
        else if (trafficModeAttr.equals("all"))
          trafficMode = TRAFFIC_MODE_ALL;
      }
    }
    archiving = routeElem.getAttribute("archiving").equalsIgnoreCase("true");
    final String billingAttr = routeElem.getAttribute("billing");
    if (billingAttr.equalsIgnoreCase("true"))
      billing = BILLING_TRUE;
    else if (billingAttr.equalsIgnoreCase("fr"))
      billing = BILLING_FR;
    else if (billingAttr.equalsIgnoreCase("mt"))
      billing = BILLING_MT;
    else if (billingAttr.equalsIgnoreCase("onsubmit"))
      billing = BILLING_ON_SUBMIT;
    else if (billingAttr.equalsIgnoreCase("cdr"))
      billing = BILLING_CDR;
    else
      billing = BILLING_FALSE;
    transit = routeElem.getAttribute("transit").equalsIgnoreCase("true");
    serviceId = Integer.decode(routeElem.getAttribute("serviceId")).intValue();
    suppressDeliveryReports = Boolean.valueOf(routeElem.getAttribute("suppressDeliveryReports")).booleanValue();
    active = Boolean.valueOf(routeElem.getAttribute("active")).booleanValue();
    srcSmeId = routeElem.getAttribute("srcSmeId");
    deliveryMode = routeElem.getAttribute("deliveryMode");
    forwardTo = routeElem.getAttribute("forwardTo");
    hide = routeElem.getAttribute("hide").length() <= 0 || routeElem.getAttribute("hide").equalsIgnoreCase("true");
    replayPath = getReplayPathValue(routeElem.getAttribute("replayPath"));
    notes = "";
    final NodeList notesList = routeElem.getElementsByTagName("notes");
    for (int i = 0; i < notesList.getLength(); i++)
      notes += Utils.getNodeText(notesList.item(i));
    forceDelivery = Boolean.valueOf(routeElem.getAttribute("forceDelivery")).booleanValue();
    final String aclIdStr = routeElem.getAttribute("aclId");
    aclId = aclIdStr != null && aclIdStr.trim().length() > 0 ? Long.decode(aclIdStr).longValue() : -1;
    allowBlocked = Boolean.valueOf(routeElem.getAttribute("allowBlocked")).booleanValue();
    final String providerIdStr = routeElem.getAttribute("providerId");
    providerId = providerIdStr != null && providerIdStr.trim().length() > 0 ? Long.decode(providerIdStr).longValue() : -1;
    final String categoryIdStr = routeElem.getAttribute("categoryId");
    categoryId = categoryIdStr != null && categoryIdStr.trim().length() > 0 ? Long.decode(categoryIdStr).longValue() : -1;
    backupSmeId = routeElem.getAttribute("backupSme");
  }

  public static byte getReplayPathValue(final String replayPathStr)
  {
    if (replayPathStr == null || replayPathStr.length() == 0)
      return REPLAY_PATH_PASS;
    if (replayPathStr.equalsIgnoreCase("force"))
      return REPLAY_PATH_FORCE;
    if (replayPathStr.equalsIgnoreCase("suppress"))
      return REPLAY_PATH_SUPPRESS;
    if (replayPathStr.equalsIgnoreCase("pass"))
      return REPLAY_PATH_PASS;

    return REPLAY_PATH_PASS;
  }

  public static String getReplayPathValue(final byte replayPath)
  {
    switch (replayPath) {
      case REPLAY_PATH_FORCE:
        return "force";
      case REPLAY_PATH_SUPPRESS:
        return "suppress";
      case REPLAY_PATH_PASS:
        return "pass";
      default:
        return "pass";
    }
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH)
      throw new IllegalArgumentException("Route name is too long");
    this.name = name;
  }

  public void addSource(final Source newSrc)
  {
    if (newSrc == null)
      throw new NullPointerException("Source is null");

    src.add(newSrc);
  }

  public Source removeSource(final String sourceName)
  {
    return src.remove(sourceName);
  }

  public Destination removeDestination(final String destinationName)
  {
    return dst.remove(destinationName);
  }

  public void addDestination(final Destination newDst)
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

  public void updateSources(final Set sourcesStrings, final String masksString, final SubjectList allSubjects)
  {
    final SourceList source_selected = new SourceList();
    for (Iterator i = sourcesStrings.iterator(); i.hasNext();) {
      final String token = (String) i.next();
      source_selected.add(new Source(allSubjects.get(token)));
    }

    final MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      final Mask m = (Mask) i.next();
      source_selected.add(new Source(m));
    }

    src.retainAll(source_selected);
    source_selected.removeAll(src);
    src.addAll(source_selected);
  }

  public void updateDestinations(final Set destinationsStrings, final String masksString, final SubjectList allSubjects, final SME defaultSme)
          throws AdminException
  {
    final DestinationList list = new DestinationList();
    for (Iterator i = destinationsStrings.iterator(); i.hasNext();) {
      final String token = (String) i.next();
      list.add(new Destination(allSubjects.get(token)));
    }

    final MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      final Mask m = (Mask) i.next();
      list.add(new Destination(m, defaultSme));
    }

    dst.retainAll(list);
    list.removeAll(dst);
    dst.addAll(list);
  }

  public int getTrafficMode() {
    return trafficMode;
  }

  public void setTrafficMode(int trafficMode) {
    this.trafficMode = trafficMode;
  }

  public String getBillingString()
  {
    switch( billing ) {
      case BILLING_FALSE: return "false";
      case BILLING_TRUE: return "true";
      case BILLING_MT: return "mt";
      case BILLING_FR: return "fr";
      case BILLING_ON_SUBMIT: return "onsubmit";
      case BILLING_CDR: return "cdr";
      default: return "false";
    }
  }

  public byte getBilling()
  {
    return billing;
  }

  public void setBilling(final byte billing)
  {
    this.billing = billing;
  }

  public boolean isTransit()
  {
    return transit;
  }

  public void setTransit(boolean transit)
  {
    this.transit = transit;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(final boolean archiving)
  {
    this.archiving = archiving;
  }

  public PrintWriter store(final PrintWriter out)
  {
    String trafficModeStr = "none";
    switch (trafficMode) {
      case TRAFFIC_MODE_ALL: trafficModeStr="all"; break;
      case TRAFFIC_MODE_USSD: trafficModeStr="ussdOnly"; break;
      case TRAFFIC_MODE_SMS: trafficModeStr="smsOnly"; break;
      default: trafficModeStr = "none";
    }

    out.print("  <route id=\"" + StringEncoderDecoder.encode(getName())
            + "\" billing=\"" + getBillingString()
            + "\" transit=\"" + isTransit()
            + "\" archiving=\"" + isArchiving()
            + "\" trafficMode=\"" + trafficModeStr
            + "\" priority=\"" + getPriority()
            + "\" serviceId=\"" + getServiceId()
            + "\" suppressDeliveryReports=\"" + isSuppressDeliveryReports()
            + "\" active=\"" + isActive()
            + "\" srcSmeId=\"" + StringEncoderDecoder.encode(getSrcSmeId())
            + "\" deliveryMode=\"" + StringEncoderDecoder.encode(getDeliveryMode())
            + "\" hide=\"" + isHide()
            + "\" replayPath=\"" + getReplayPathValue(getReplayPath())
            + "\" forceDelivery=\"" + isForceDelivery()
            + ("MAP_PROXY".equals(getSrcSmeId()) ? "\" forwardTo=\"" + StringEncoderDecoder.encode(getForwardTo()) : "")
            + "\" aclId=\"" + getAclId()
            + "\" allowBlocked=\"" + isAllowBlocked()
            + "\" providerId=\"" + getProviderId()
            + "\" categoryId=\"" + getCategoryId() + "\"");
    if (backupSmeId != null && backupSmeId.trim().length() > 0)
      out.print(" backupSme=\"" + backupSmeId + "\"");
    out.println(">");

    if (notes != null)
      out.println("    <notes>" + StringEncoderDecoder.encode(notes) + "</notes>");
    getSources().store(out);
    getDestinations().store(out);
    out.println("  </route>");
    return out;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(final int priority)
  {
    this.priority = priority;
  }

  public int getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(final int serviceId)
  {
    this.serviceId = serviceId;
  }

  public boolean isSuppressDeliveryReports()
  {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(final boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(final boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(final String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getDeliveryMode()
  {
    return deliveryMode;
  }

  public void setDeliveryMode(final String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(final String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(final boolean hide)
  {
    this.hide = hide;
  }

  public byte getReplayPath()
  {
    return replayPath;
  }

  public void setReplayPath(final byte replayPath)
  {
    this.replayPath = replayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(final String notes)
  {
    this.notes = notes;
  }

  public boolean isForceDelivery()
  {
    return forceDelivery;
  }

  public void setForceDelivery(final boolean forceDelivery)
  {
    this.forceDelivery = forceDelivery;
  }

  public long getAclId()
  {
    return aclId;
  }

  public void setAclId(final long aclId)
  {
    this.aclId = aclId;
  }

  public boolean isAllowBlocked()
  {
    return allowBlocked;
  }

  public void setAllowBlocked(final boolean allowBlocked)
  {
    this.allowBlocked = allowBlocked;
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(final long providerId)
  {
    this.providerId = providerId;
  }

  public long getCategoryId()
  {
    return categoryId;
  }

  public void setCategoryId(final long categoryId)
  {
    this.categoryId = categoryId;
  }

  public String getBackupSmeId()
  {
    return backupSmeId;
  }

  public void setBackupSmeId(String backupSmeId)
  {
    this.backupSmeId = backupSmeId;
  }
}
