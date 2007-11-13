package ru.novosoft.smsc.jsp.smsc.snmp;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.PrintWriter;
import java.util.*;

public class SnmpObject {
    public String id = "";
    public boolean enabled = false;
    public Map counters = new HashMap();
    public Map severities = new HashMap();

    public SnmpObject() {
    }

    public SnmpObject(Element elem) {
        id = elem.getAttribute("id");
        enabled = elem.getAttribute("enabled").equalsIgnoreCase("true");

        NodeList counterNodes = elem.getElementsByTagName("counter");
        for (int i = 0; i < counterNodes.getLength(); i++) {
            Element cnt = (Element) counterNodes.item(i);
            String name = cnt.getAttribute("name");
            String value = cnt.getFirstChild().getNodeValue();
            StringTokenizer tkn = new StringTokenizer(value, ",");
            Vector limits = new Vector();
            while (tkn.hasMoreElements())
                limits.add(tkn.nextToken());
            counters.put(name, limits);
        }

        final NodeList severityNodes = elem.getElementsByTagName("severity");
        Element svrt;
        for (int i = 0; i < severityNodes.getLength(); i++) {
          svrt = (Element) severityNodes.item(i);
          severities.put(svrt.getAttribute("event"), svrt.getAttribute("value"));
        }
    }

    public static void setCounter(Map counters, String cntName, String cntLimit, String cntValue) throws Exception {
        Vector limits = (Vector) counters.get(cntName);
        if (limits == null) {
            limits = new Vector();
            limits.setSize(4);
            counters.put(cntName, limits);
        }
        int i = -1;

        if (cntLimit.equals("warning")) i = 0;
        else if (cntLimit.equals("minor")) i = 1;
        else if (cntLimit.equals("major")) i = 2;
        else if (cntLimit.equals("critical")) i = 3;

        if (i != -1) {
            limits.removeElementAt(i);
            limits.insertElementAt(cntValue, i);
        }
    }

    public static void writeCounters(PrintWriter out, Map counters) throws Exception {
        for (Iterator i = counters.keySet().iterator(); i.hasNext();) {
            String name = (String) i.next();
            out.print("       <counter name=\"" + name + "\">");
            Vector vct = (Vector) counters.get(name);
            out.print((String) vct.elementAt(0) + "," + (String)vct.elementAt(1) + "," + (String)vct.elementAt(2) + "," + (String)vct.elementAt(3));
            out.println("</counter>");
        }
    }

    public static void writeSeverities(PrintWriter out, Map severities) throws Exception {
      for (Iterator i = severities.keySet().iterator(); i.hasNext();) {
            String name = (String) i.next();
            out.println("       <severity event=\"" + name + "\" value=\"" + severities.get(name) + "\"/>");
        }
    }
}
