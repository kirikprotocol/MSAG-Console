package ru.novosoft.smsc.jsp.smsc.snmp;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.xml.Utils;

import javax.servlet.http.HttpServletRequest;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

public class ConfigBean extends PageBean {
    public static final String DEFAULT_SECTION = "default";
    public static final String COUNTER_INTERVAL = "counterInterval";
    public static final String OBJECT_SECTION = "object";

    public static final Collection SEVERITY_VALUES;
    public static final Collection SEVERITY_NAMES;

    static {
      SEVERITY_VALUES = new LinkedList();
      SEVERITY_VALUES.add("off");
      SEVERITY_VALUES.add("normal");
      SEVERITY_VALUES.add("warning");
      SEVERITY_VALUES.add("minor");
      SEVERITY_VALUES.add("major");
      SEVERITY_VALUES.add("critical");

      SEVERITY_NAMES = new LinkedList();
      SEVERITY_NAMES.add("register");
      SEVERITY_NAMES.add("registerFailed");
      SEVERITY_NAMES.add("unregister");
      SEVERITY_NAMES.add("unregisterFailed");
    }

    private String mbSave = null;
    private String mbReset = null;
    private Map params = new HashMap();
    private int counterInterval = 0;

    private File config = null;

    public Map defaultCounters = new HashMap();
    private boolean defaultEnabled = false;
    public Map defaultSeverities = new HashMap();

    public Map objects = new HashMap();

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK) return result;
        return result;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        try {
            config = new File(WebAppFolders.getSmscConfFolder(), "snmp.xml");
        } catch (Exception e) {
            return error(SMSCErrors.error.snmp.CouldntGetConfig, e);
        }


        try {
          processParams(request);
        } catch (Exception e) {
            logger.error("unable to parse SNMP config", e);
            return error(SMSCErrors.error.smsc.couldntSave, e);
        }

        if (mbSave != null)
            return processApply();
        else if (mbReset != null)
            return processReset();

        return RESULT_OK;
    }

    private int processApply() {
        try {
            objects.clear();
            defaultCounters.clear();
            defaultSeverities.clear();
            for (Iterator i = params.keySet().iterator(); i.hasNext();) {
                String name = (String) i.next();
                String value = (String) params.get(name);
                StringTokenizer tkn = new StringTokenizer(name, ".");
                String firstToken = tkn.nextToken();
                String secondToken = tkn.nextToken();

                if (firstToken.endsWith("_counters")) {
                    firstToken = firstToken.substring(0, firstToken.lastIndexOf("_counters"));
                }

                if (secondToken.equals("counter")) {
                  Map counters = null;
                  if (firstToken.equals("default"))
                    counters = defaultCounters;
                  else {
                    SnmpObject obj = (SnmpObject) objects.get(firstToken);
                    if (obj == null) {
                      obj = new SnmpObject();
                      obj.id = firstToken;
                      objects.put(firstToken, obj);
                    }
                    counters = obj.counters;
                  }
                  System.out.println(name);
                  String thirdToken = tkn.nextToken();
                  String fourthToken = tkn.nextToken();

                  SnmpObject.setCounter(counters, thirdToken, fourthToken, value);

                } else if (secondToken.equals("enabled")) {
                  SnmpObject obj = ((SnmpObject) objects.get(firstToken));
                  if (obj == null) {
                    obj = new SnmpObject();
                    obj.id = firstToken;
                    objects.put(firstToken, obj);
                  }
                  obj.enabled = Boolean.valueOf(value).booleanValue();

                } else if (secondToken.equals("severity")) {
                  Map severities = null;
                  if (firstToken.equals("default"))
                    severities = defaultSeverities;
                  else {
                    SnmpObject obj = (SnmpObject) objects.get(firstToken);
                    if (obj == null) {
                      obj = new SnmpObject();
                      obj.id = firstToken;
                      objects.put(firstToken, obj);
                    }
                    severities = obj.severities;
                  }
                  String thirdToken = tkn.nextToken();
                  severities.put(thirdToken, value);
                }

            }
            appContext.getStatuses().setSNMPChanged(true);
            journalAppend(SubjectTypes.TYPE_snmp, "config changed", Actions.ACTION_MODIFY);
            return save();
        } catch (Exception e) {
            logger.error("Couldn't save SNMP config", e);
            return error(SMSCErrors.error.smsc.couldntSave);
        }
    }

    private int processReset() {
        params.clear();
        counterInterval = 0;
        try {
            return loadParams();
        } catch (Exception e) {
            logger.error("unable to parse SNMP config", e);
            return error(SMSCErrors.error.smsc.couldntSave);
        }
    }

    private int save() throws Exception {
        final File newFile = Functions.createNewFilenameForSave(config);
        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "config", "snmp.dtd", "WINDOWS-1251"/*Functions.getLocaleEncoding()*/);
        out.println("   <counterInterval value=\"" + String.valueOf(counterInterval) + "\"/>");
        out.println("   <default enabled=\"" + (defaultEnabled) + "\">");
        SnmpObject.writeCounters(out, defaultCounters);
        SnmpObject.writeSeverities(out, defaultSeverities);
        out.println("   </default>");
        for (Iterator i = objects.keySet().iterator(); i.hasNext();) {
            String name = (String) i.next();
            SnmpObject obj = (SnmpObject) objects.get(name);
            out.println("   <object id=\"" + obj.id + "\" enabled=\"" + Boolean.toString(obj.enabled) + "\">");
            SnmpObject.writeCounters(out, obj.counters);
            SnmpObject.writeSeverities(out, obj.severities);
            out.println("   </object>");
        }
        Functions.storeConfigFooter(out, "config");
        out.flush();
        out.close();
        Functions.renameNewSavedFileToOriginal(newFile, config);
        return RESULT_OK;
    }

    private int loadParams() throws AdminException {
        try {
            Document doc = Utils.parse(config.getAbsolutePath());

            // Load counter interval
            if (counterInterval == 0) {
              try {
                final NodeList counterInt = doc.getElementsByTagName(COUNTER_INTERVAL);
                if (counterInt.getLength() > 0)
                  counterInterval = Integer.parseInt(((Element)counterInt.item(0)).getAttribute("value"));
              } catch (NumberFormatException e) {
                logger.error("Invalid counterInterval value");
                counterInterval = 0;
              }
            }

            // Load default
            NodeList a = doc.getElementsByTagName(DEFAULT_SECTION);
            if (a.getLength() > 0) {
                Element defaultNode = ((Element) a.item(0));
                defaultEnabled = defaultNode.getAttribute("enabled") != null && defaultNode.getAttribute("enabled").equalsIgnoreCase("true");

                final Element elem = (Element) a.item(0);
                NodeList defaultCounterNodes = elem.getElementsByTagName("counter");
                for (int i = 0; i < defaultCounterNodes.getLength(); i++) {
                    Element cnt = (Element) defaultCounterNodes.item(i);
                    String name = cnt.getAttribute("name");
                    String value = cnt.getFirstChild().getNodeValue();
                    StringTokenizer tkn = new StringTokenizer(value, ",");
                    Vector limits = new Vector();
                    while (tkn.hasMoreElements())
                        limits.add(tkn.nextToken());
                    defaultCounters.put(name, limits);
                }

                final NodeList severityNodes = elem.getElementsByTagName("severity");
                Element cnt;
                for (int i = 0; i < severityNodes.getLength(); i++) {
                  cnt = (Element) severityNodes.item(i);
                  defaultSeverities.put(cnt.getAttribute("event"), cnt.getAttribute("value"));
                }
            }

            // Load objects
            a = doc.getElementsByTagName(OBJECT_SECTION);
            for (int i = 0; i < a.getLength(); i++) {
                SnmpObject obj = new SnmpObject((Element) a.item(i));
                objects.put(obj.id, obj);
            }


        } catch (FactoryConfigurationError error) {
            logger.error("Couldn't configure xml parser factory", error);
            throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
        } catch (ParserConfigurationException e) {
            logger.error("Couldn't configure xml parser", e);
            throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
        } catch (SAXException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't perform IO operation", e);
            throw new AdminException("Couldn't perform IO operation: " + e.getMessage());
        } catch (NullPointerException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        }

        return RESULT_OK;
    }

    private int processParams(HttpServletRequest request) throws AdminException {
        int result = RESULT_OK;
        if (params.size() == 0)
          loadParams();

        Enumeration parameterNames = request.getParameterNames();
        defaultEnabled = request.getParameter("defaultEnabled") != null;

        while (parameterNames.hasMoreElements()) {
            String s = (String) parameterNames.nextElement();
            if (s.indexOf('.') <= 0 || s.startsWith("newParamName_") || s.startsWith("newParamValue_")) continue;
            if (s.indexOf('.') <= 0 || s.startsWith("newSeverityInput_")) continue;
            if (s.indexOf('.') <= 0 || s.startsWith("newSectionCounterInput_")) continue;

            final String parameter = request.getParameter(s);
            if (parameter.trim().length() > 0)
                params.put(s, parameter);
        }

        return result;
    }

  public String getCounterInterval() {
    return String.valueOf(counterInterval);
  }

  public void setCounterInterval(String counterInterval) {
    try {
      this.counterInterval = Integer.parseInt(counterInterval);
    } catch (NumberFormatException e) {
    }
  }

  public boolean isDefaultEnabled() {
    return defaultEnabled;
  }

  public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getMbReset() {
        return mbReset;
    }

    public void setMbReset(String mbReset) {
        this.mbReset = mbReset;
    }
}
