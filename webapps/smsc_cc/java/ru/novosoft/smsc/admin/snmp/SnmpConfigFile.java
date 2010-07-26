package ru.novosoft.smsc.admin.snmp;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

/**
 * @author Artem Snopkov
 */
class SnmpConfigFile implements ManagedConfigFile {

  private static final String DEFAULT_SECTION = "default";
  private static final String COUNTER_INTERVAL = "counterInterval";
  private static final String OBJECT_SECTION = "object";

  private int counterInterval;

  private SnmpObject defaultSnmpObject;
  private Map<String, SnmpObject> snmpObjects = new HashMap<String, SnmpObject>();

  public int getCounterInterval() {
    return counterInterval;
  }

  public void setCounterInterval(int counterInterval) {
    if (counterInterval < 0)
      throw new IllegalArgumentException("counterInterval");
    this.counterInterval = counterInterval;
  }

  public SnmpObject getDefaultSnmpObject() {
    return defaultSnmpObject;
  }

  public void setDefaultSnmpObject(SnmpObject defaultSnmpObject) {
    if (defaultSnmpObject == null)
      throw new NullPointerException();
    this.defaultSnmpObject = defaultSnmpObject;
  }

  public Map<String, SnmpObject> getSnmpObjects() {
    return snmpObjects;
  }

  public void setSnmpObjects(Map<String, SnmpObject> snmpObjects) {
    if (snmpObjects == null)
      throw new NullPointerException();
    this.snmpObjects = snmpObjects;
  }

  private static void writeSnmpCounter(String name, SnmpCounter counter, PrintWriter out) {
    out.print("<counter name=\"");
    out.print(name);
    out.print("\">");
    out.print(counter.getWarning());
    out.print(",");
    out.print(counter.getMinor());
    out.print(",");
    out.print(counter.getMajor());
    out.print(",");
    out.print(counter.getCritical());
    out.println("</counter>");
  }

  private static void writeSeverity(String name, SnmpSeverity severity, PrintWriter out) {
    out.print("<severity event=\"");
    out.print(name);
    out.print("\" value=\"");
    switch (severity) {
      case OFF: out.print("off");break;
      case NORMAL: out.print("normal");break;
      case WARNING: out.print("warning");break;
      case MAJOR: out.print("major");break;
      case MINOR: out.print("minor");break;
      case CRITICAL: out.print("critical");break;
    }
    out.print("\"/>");
  }

  private static void writeSnmpObject(SnmpObject o, PrintWriter out) {
    if (o.getCounterOther() != null)
      writeSnmpCounter("other", o.getCounterOther(), out);
    if (o.getCounter0014() != null)
      writeSnmpCounter("0x014", o.getCounter0014(), out);
    if (o.getCounter000d() != null)
      writeSnmpCounter("0x00d", o.getCounter000d(), out);
    if (o.getCounterFailed() != null)
      writeSnmpCounter("failed", o.getCounterFailed(), out);
    if (o.getCounterAccepted() != null)
      writeSnmpCounter("accepted", o.getCounterAccepted(), out);
    if (o.getCounterRejected() != null)
      writeSnmpCounter("rejected", o.getCounterRejected(), out);
    if (o.getCounterDelivered() != null)
      writeSnmpCounter("delivered", o.getCounterDelivered(), out);
    if (o.getCounterSDP() != null)
      writeSnmpCounter("SDP", o.getCounterSDP(), out);
    if (o.getCounterRetried() != null)
      writeSnmpCounter("retried", o.getCounterRetried(), out);
    if (o.getCounter0058() != null)
      writeSnmpCounter("0x058", o.getCounter0058(), out);
    if (o.getCounterTempError() != null)
      writeSnmpCounter("temperror", o.getCounterTempError(), out);

    if(o.getSeverityRegister() != null)
      writeSeverity("register", o.getSeverityRegister(), out);
    if(o.getSeverityUnregister() != null)
      writeSeverity("unregister", o.getSeverityUnregister(), out);
    if(o.getSeverityRegisterFailed() != null)
      writeSeverity("registerFailed", o.getSeverityRegisterFailed(), out);
    if(o.getSeverityUnregisterFailed() != null)
      writeSeverity("unregisterFailed", o.getSeverityUnregisterFailed(), out);
  }

  public void save(InputStream oldFile, OutputStream newFile) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(newFile));
      XmlUtils.storeConfigHeader(out, "config", "snmp.dtd", "WINDOWS-1251"/*Functions.getLocaleEncoding()*/);
      out.println("   <counterInterval value=\"" + String.valueOf(counterInterval) + "\"/>");
      out.println("   <default enabled=\"" + (defaultSnmpObject.isEnabled()) + "\">");
      writeSnmpObject(defaultSnmpObject, out);
      out.println("   </default>");
      for (Map.Entry<String, SnmpObject> e : snmpObjects.entrySet()) {
        String name = e.getKey();
        SnmpObject obj = e.getValue();
        out.println("   <object id=\"" + name + "\" enabled=\"" + Boolean.toString(obj.isEnabled()) + "\">");
        writeSnmpObject(obj, out);
        out.println("   </object>");
      }
      XmlUtils.storeConfigFooter(out, "config");
    } finally {
      if (out != null)
        out.close();
    }
  }

  private static SnmpCounter parseCounter(String counterValue) throws AdminException {
    StringTokenizer st = new StringTokenizer(counterValue, ",");
    int warning = Integer.parseInt(st.nextToken());
    int minor = Integer.parseInt(st.nextToken());
    int major = Integer.parseInt(st.nextToken());
    int critical = Integer.parseInt(st.nextToken());
    return new SnmpCounter(warning, minor, major, critical);
  }

  private static SnmpSeverity parseSeverity(String severityValue) {
    if (severityValue.equals("off"))
      return SnmpSeverity.OFF;
    if (severityValue.equals("normal"))
      return SnmpSeverity.NORMAL;
    if (severityValue.equals("warning"))
      return SnmpSeverity.WARNING;
    if (severityValue.equals("minor"))
      return SnmpSeverity.MINOR;
    if (severityValue.equals("major"))
      return SnmpSeverity.MAJOR;
    if (severityValue.equals("critical"))
      return SnmpSeverity.CRITICAL;
    return null;
  }

  private static void parseSnmpObject(SnmpObject o, Element elem) throws AdminException {

    NodeList defaultCounterNodes = elem.getElementsByTagName("counter");

    // Parse counters
    for (int i = 0; i < defaultCounterNodes.getLength(); i++) {
      Element cnt = (Element) defaultCounterNodes.item(i);
      String counterName = cnt.getAttribute("name");
      String counterValue = cnt.getFirstChild().getNodeValue();

      SnmpCounter c = parseCounter(counterValue);

      if (counterName.equals("other")) {
        o.setCounterOther(c);
      } else if (counterName.equals("0x014")) {
        o.setCounter0014(c);
      } else if (counterName.equals("0x00d")) {
        o.setCounter000d(c);
      } else if (counterName.equals("failed")) {
        o.setCounterFailed(c);
      } else if (counterName.equals("accepted")) {
        o.setCounterAccepted(c);
      } else if (counterName.equals("rejected")) {
        o.setCounterRejected(c);
      } else if (counterName.equals("delivered")) {
        o.setCounterDelivered(c);
      } else if (counterName.equals("SDP")) {
        o.setCounterSDP(c);
      } else if (counterName.equals("retried")) {
        o.setCounterRetried(c);
      } else if (counterName.equals("0x058")) {
        o.setCounter0058(c);
      } else if (counterName.equals("temperror")) {
        o.setCounterTempError(c);
      }
    }

    // Parse severities
    final NodeList severityNodes = elem.getElementsByTagName("severity");
    Element cnt;
    for (int i = 0; i < severityNodes.getLength(); i++) {
      cnt = (Element) severityNodes.item(i);
      String severityName = cnt.getAttribute("event");
      String severityValue = cnt.getAttribute("value");

      SnmpSeverity s = parseSeverity(severityValue);

      if (severityName.equals("register")) {
        o.setSeverityRegister(s);
      } else if (severityName.equals("unregister")) {
        o.setSeverityUnregister(s);
      } else if (severityName.equals("registerFailed")) {
        o.setSeverityRegisterFailed(s);
      } else if (severityName.equals("unregisterFailed")) {
        o.setSeverityUnregisterFailed(s);
      }
    }
  }

  public void load(InputStream is) throws Exception {

    snmpObjects.clear();

    Document doc = XmlUtils.parse(is);

    // Load counter interval
    if (counterInterval == 0) {
      final NodeList counterInt = doc.getElementsByTagName(COUNTER_INTERVAL);
      if (counterInt.getLength() > 0)
        counterInterval = Integer.parseInt(((Element) counterInt.item(0)).getAttribute("value"));
    }

    // Load default
    defaultSnmpObject = new SnmpObject();
    NodeList a = doc.getElementsByTagName(DEFAULT_SECTION);
    if (a.getLength() > 0) {
      Element defaultNode = ((Element) a.item(0));
      boolean defaultEnabled = defaultNode.getAttribute("enabled") != null && defaultNode.getAttribute("enabled").equalsIgnoreCase("true");

      defaultSnmpObject.setEnabled(defaultEnabled);

      parseSnmpObject(defaultSnmpObject, defaultNode);
    }

    // Load objects
    a = doc.getElementsByTagName(OBJECT_SECTION);
    for (int i = 0; i < a.getLength(); i++) {
      Element node = ((Element) a.item(i));
      boolean enabled = node.getAttribute("enabled") != null && node.getAttribute("enabled").equalsIgnoreCase("true");
      String objectId = node.getAttribute("id");

      SnmpObject obj = new SnmpObject();
      obj.setEnabled(enabled);
      parseSnmpObject(obj, node);

      snmpObjects.put(objectId, obj);
    }
  }
}
