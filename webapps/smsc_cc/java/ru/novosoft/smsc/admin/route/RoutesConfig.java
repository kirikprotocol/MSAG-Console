package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class RoutesConfig implements ManagedConfigFile<RouteSubjectSettings> {

  private final RouteSubjectManager manager;

  RoutesConfig(RouteSubjectManager manager) {
    this.manager = manager;
  }

  public void save(InputStream oldFile, OutputStream newFile, RouteSubjectSettings conf) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(newFile, Functions.getLocaleEncoding()));
      XmlUtils.storeConfigHeader(out, "routes", "routes.dtd", Functions.getLocaleEncoding());

      storeSubjects(conf.getSubjects(), out);
      storeRoutes(conf.getRoutes(), out);

      XmlUtils.storeConfigFooter(out, "routes");

    } finally {
      if (out != null)
        out.close();
    }
  }

  private static void storeSubjects(List<Subject> subjects, PrintWriter out) {
    for (Subject s : subjects) {
      out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(s.getName()) + "\" defSme=\"" + StringEncoderDecoder.encode(s.getDefaultSmeId()) + "\">");
      if (s.getNotes() != null)
        out.println("    <notes>" + s.getNotes() + "</notes>");

      if (s.getMasks() != null) {
        for (Address mask : s.getMasks())
          out.println("       <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedAddress()) + "\"/>");
      }

      if (s.getChildren() != null) {
        for (String child : s.getChildren())
          out.println("    <subject id=\"" + child + "\"/>");
      }
      out.println("  </subject_def>");
    }
  }

  private static void storeRoutes(List<Route> routes, PrintWriter out) {
    for (Route r : routes) {
      out.print("  <route id=\"" + StringEncoderDecoder.encode(r.getName())
          + "\" billing=\"" + getBillingString(r.getBilling())
          + "\" transit=\"" + r.isTransit()
          + "\" archiving=\"" + r.isArchiving()
          + "\" enabling=\"" + r.isEnabling()
          + "\" priority=\"" + r.getPriority()
          + "\" serviceId=\"" + r.getServiceId()
          + "\" suppressDeliveryReports=\"" + r.isSuppressDeliveryReports()
          + "\" active=\"" + r.isActive()
          + "\" srcSmeId=\"" + StringEncoderDecoder.encode(r.getSrcSmeId())
          + "\" deliveryMode=\"" + getDeliveryMode(r.getDeliveryMode())
          + "\" hide=\"" + r.isHide()
          + "\" replayPath=\"" + getReplayPathValue(r.getReplayPath())
          + "\" forceDelivery=\"" + r.isForceDelivery()
          + ("MAP_PROXY".equals(r.getSrcSmeId()) ? "\" forwardTo=\"" + StringEncoderDecoder.encode(r.getForwardTo()) : "")
          + "\" aclId=\"" + r.getAclId()
          + "\" allowBlocked=\"" + r.isAllowBlocked()
          + "\" providerId=\"" + r.getProviderId()
          + "\" categoryId=\"" + r.getCategoryId() + "\"");
      if (r.getBackupSmeId() != null && r.getBackupSmeId().trim().length() > 0)
        out.print(" backupSme=\"" + r.getBackupSmeId() + "\"");
      out.println(">");

      if (r.getNotes() != null)
        out.println("    <notes>" + StringEncoderDecoder.encode(r.getNotes()) + "</notes>");

      if (r.getSources() != null) {
        for (Source s : r.getSources()) {
          out.println("    <source>");
          if (s.getSubject() != null)
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(s.getSubject()) + "\"/>");
          else
            out.println("      <mask value=\"" + StringEncoderDecoder.encode(s.getMask().getNormalizedAddress()) + "\"/>");
          out.println("    </source>");
        }
      }

      if (r.getDestinations() != null) {
        for (Destination d : r.getDestinations()) {
          out.println("    <destination sme=\"" + StringEncoderDecoder.encode(d.getSmeId()) + "\">");
          if (d.getSubject() != null)
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(d.getSubject()) + "\"/>");
          else
            out.println("      <mask value=\"" + StringEncoderDecoder.encode(d.getMask().getNormalizedAddress()) + "\"/>");
          out.println("    </destination>");
        }
      }
      out.println("  </route>");
    }
  }

  private static String getDeliveryMode(DeliveryMode mode) {
    switch (mode) {
      case DEFAULT:
        return "default";
      case STORE_AND_FORWARD:
        return "store and forward";
      case FORWARD:
        return "forward";
      case DATAGRAM:
        return "datagram";
      default:
        return "default";
    }
  }

  private static String getBillingString(BillingMode billing) {
    if (billing == null)
      return "false";
    switch (billing) {
      case ON:
        return "true";
      case MT:
        return "mt";
      case FR:
        return "fr";
      case ON_SUBMIT:
        return "onsubmit";
      case CDR:
        return "cdr";
      default:
        return "false";
    }
  }

  private static String getReplayPathValue(ReplayPath replayPath) {
    switch (replayPath) {
      case FORCE:
        return "force";
      case SUPPRESS:
        return "suppress";
      case PASS:
        return "pass";
      default:
        return "pass";
    }
  }


  public RouteSubjectSettings load(InputStream is) throws Exception {
    Document d = XmlUtils.parse(is);
    List<Subject> subjects = parseSubjects(d.getDocumentElement());
    List<Route> routes = parseRoutes(d.getDocumentElement());
    RouteSubjectSettings s = new RouteSubjectSettings(manager);
    s.setSubjects(subjects);
    s.setRoutes(routes);
    return s;
  }

  private static List<Subject> parseSubjects(Element listElement) throws AdminException {

    List<Subject> subjects = new ArrayList<Subject>();

    NodeList subjList = listElement.getElementsByTagName("subject_def");
    for (int i = 0; i < subjList.getLength(); i++) {
      Element subjElem = (Element) subjList.item(i);
      String name = subjElem.getAttribute("id");

      NodeList masksList = subjElem.getElementsByTagName("mask");
      List<Address> masks = new ArrayList<Address>(masksList.getLength());
      for (int j = 0; j < masksList.getLength(); j++) {
        Element maskElem = (Element) masksList.item(j);
        masks.add(new Address(maskElem.getAttribute("value").trim()));
      }

      NodeList childList = subjElem.getElementsByTagName("subject");
      List<String> childs = new ArrayList<String>(childList.getLength());
      for (int j = 0; j < childList.getLength(); j++) {
        Element childElem = (Element) childList.item(j);
        childs.add(childElem.getAttribute("id").trim());
      }

      String defSme = subjElem.getAttribute("defSme");

      String notes = "";
      NodeList notesList = subjElem.getElementsByTagName("notes");
      for (int j = 0; j < notesList.getLength(); j++)
        notes += XmlUtils.getNodeText(notesList.item(j));

      Subject s = new Subject(name);
      s.setChildren(childs);
      s.setDefaultSmeId(defSme);
      s.setMasks(masks);
      s.setNotes(notes);

      subjects.add(s);
    }

    return subjects;
  }

  private static List<Route> parseRoutes(Element routeListElement) throws AdminException {
    List<Route> routes = new ArrayList<Route>();

    NodeList routeList = routeListElement.getElementsByTagName("route");
    for (int i = 0; i < routeList.getLength(); i++) {
      Element routeElem = (Element) routeList.item(i);
      routes.add(parseRoute(routeElem));
    }

    return routes;
  }

  private static Route parseRoute(Element routeElem) throws AdminException {
    String name = routeElem.getAttribute("id");
    Route r = new Route(name);
    r.setPriority(Integer.parseInt(routeElem.getAttribute("priority")));
    r.setEnabling(routeElem.getAttribute("enabling").equalsIgnoreCase("true"));
    r.setArchiving(routeElem.getAttribute("archiving").equalsIgnoreCase("true"));
    final String billingAttr = routeElem.getAttribute("billing");
    r.setBilling(parseBillingMode(billingAttr));
    if (billingAttr.equalsIgnoreCase("true"))
      r.setBilling(BillingMode.ON);
    else if (billingAttr.equalsIgnoreCase("fr"))
      r.setBilling(BillingMode.FR);
    else if (billingAttr.equalsIgnoreCase("mt"))
      r.setBilling(BillingMode.MT);
    else if (billingAttr.equalsIgnoreCase("onsubmit"))
      r.setBilling(BillingMode.ON_SUBMIT);
    else if (billingAttr.equalsIgnoreCase("cdr"))
      r.setBilling(BillingMode.CDR);

    r.setTransit(routeElem.getAttribute("transit").equalsIgnoreCase("true"));
    r.setServiceId(Integer.parseInt(routeElem.getAttribute("serviceId")));
    r.setSuppressDeliveryReports(Boolean.parseBoolean(routeElem.getAttribute("suppressDeliveryReports")));
    r.setActive(Boolean.valueOf(routeElem.getAttribute("active")));
    r.setSrcSmeId(routeElem.getAttribute("srcSmeId"));
    r.setDeliveryMode(parseDeliveryMode(routeElem.getAttribute("deliveryMode")));
    r.setForwardTo(routeElem.getAttribute("forwardTo"));
    r.setHide(routeElem.getAttribute("hide").length() <= 0 || routeElem.getAttribute("hide").equalsIgnoreCase("true"));
    r.setReplayPath(parseReplayPath(routeElem.getAttribute("replayPath")));
    String notes = "";
    final NodeList notesList = routeElem.getElementsByTagName("notes");
    for (int i = 0; i < notesList.getLength(); i++)
      notes += XmlUtils.getNodeText(notesList.item(i));
    r.setNotes(notes);

    r.setForceDelivery(Boolean.valueOf(routeElem.getAttribute("forceDelivery")));
    final String aclIdStr = routeElem.getAttribute("aclId");
    r.setAclId(aclIdStr != null && aclIdStr.trim().length() > 0 ? Long.decode(aclIdStr) : -1);
    r.setAllowBlocked(Boolean.valueOf(routeElem.getAttribute("allowBlocked")));
    final String providerIdStr = routeElem.getAttribute("providerId");
    r.setProviderId(providerIdStr != null && providerIdStr.trim().length() > 0 ? Long.decode(providerIdStr) : -1);
    final String categoryIdStr = routeElem.getAttribute("categoryId");
    r.setCategoryId(categoryIdStr != null && categoryIdStr.trim().length() > 0 ? Long.decode(categoryIdStr) : -1);
    r.setBackupSmeId(routeElem.getAttribute("backupSme"));

    r.setSources(parseSources(routeElem));
    r.setDestinations(parseDestinations(routeElem));

    return r;
  }

  private static BillingMode parseBillingMode(String billingAttr) {
    if (billingAttr.equalsIgnoreCase("true"))
      return BillingMode.ON;
    else if (billingAttr.equalsIgnoreCase("fr"))
      return BillingMode.FR;
    else if (billingAttr.equalsIgnoreCase("mt"))
      return BillingMode.MT;
    else if (billingAttr.equalsIgnoreCase("onsubmit"))
      return BillingMode.ON_SUBMIT;
    else if (billingAttr.equalsIgnoreCase("cdr"))
      return BillingMode.CDR;

    return null;
  }

  private static ReplayPath parseReplayPath(final String replayPathStr) {
    if (replayPathStr == null || replayPathStr.length() == 0)
      return ReplayPath.PASS;
    if (replayPathStr.equalsIgnoreCase("force"))
      return ReplayPath.FORCE;
    if (replayPathStr.equalsIgnoreCase("suppress"))
      return ReplayPath.SUPPRESS;
    if (replayPathStr.equalsIgnoreCase("pass"))
      return ReplayPath.PASS;

    return ReplayPath.PASS;
  }

  private static DeliveryMode parseDeliveryMode(String deliveryModeStr) {
    if (deliveryModeStr == null || deliveryModeStr.length() == 0)
      return DeliveryMode.DEFAULT;
    if (deliveryModeStr.equals("store and forward"))
      return DeliveryMode.STORE_AND_FORWARD;
    if (deliveryModeStr.equals("forward"))
      return DeliveryMode.FORWARD;
    if (deliveryModeStr.equals("datagram"))
      return DeliveryMode.DATAGRAM;
    return DeliveryMode.DEFAULT;
  }

  private static List<Source> parseSources(Element sourceListElement) throws AdminException {
    List<Source> sources = new ArrayList<Source>();
    NodeList list = sourceListElement.getElementsByTagName("source");
    for (int i = 0; i < list.getLength(); i++)
      sources.add(parseSource((Element) list.item(i)));
    return sources;
  }

  private static Source parseSource(Element srcElem) throws AdminException {
    NodeList list = srcElem.getElementsByTagName("subject");
    if (list.getLength() > 0) {
      Element subjElem = (Element) list.item(0);
      return new Source(subjElem.getAttribute("id"));
    } else {
      list = srcElem.getElementsByTagName("mask");
      Element maskElem = (Element) list.item(0);
      return new Source(new Address(maskElem.getAttribute("value")));
    }
  }

  private static List<Destination> parseDestinations(Element dstListElement) throws AdminException {
    List<Destination> destinations = new ArrayList<Destination>();
    NodeList list = dstListElement.getElementsByTagName("destination");
    for (int i = 0; i < list.getLength(); i++)
      destinations.add(parseDestination((Element) list.item(i)));
    return destinations;
  }

  private static Destination parseDestination(Element dstElem) throws AdminException {
    NodeList list = dstElem.getElementsByTagName("subject");
    if (list.getLength() > 0) {
      Element subjElem = (Element) list.item(0);
      return new Destination(subjElem.getAttribute("id"), dstElem.getAttribute("sme"));
    } else {
      list = dstElem.getElementsByTagName("mask");
      Element maskElem = (Element) list.item(0);
      return new Destination(new Address(maskElem.getAttribute("value")), dstElem.getAttribute("sme"));
    }
  }
}
