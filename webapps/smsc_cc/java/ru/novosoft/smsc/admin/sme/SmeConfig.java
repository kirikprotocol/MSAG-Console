package ru.novosoft.smsc.admin.sme;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.StringEncoderDecoder;
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
class SmeConfigFile implements ManagedConfigFile<Map<String, Sme>> {

  private static final Map<String, SmeBindMode> SME_BIND_MODES = new HashMap<String, SmeBindMode>();

  static {
    SME_BIND_MODES.put("tx", SmeBindMode.TX);
    SME_BIND_MODES.put("rx", SmeBindMode.RX);
    SME_BIND_MODES.put("trx", SmeBindMode.TRX);
  }

  private static void addFlag(StringBuffer sb, String flag) {
    if (sb.length() > 0)
      sb.append(',');
    sb.append(flag);
  }

  private static String getFlagsStr(Sme sme) {
    StringBuffer flags = new StringBuffer();
    if (sme.isCarryOrgDescriptor())
      addFlag(flags, "carryOrgDescriptor");
    if (sme.isCarryOrgUserInfo())
      addFlag(flags, "carryOrgUserInfo");
    if (sme.isCarrySccpInfo())
      addFlag(flags, "carrySccpInfo");
    if (sme.isFillExtraDescriptor())
      addFlag(flags, "fillExtraDescriptor");
    if (sme.isForceSmeReceipt())
      addFlag(flags, "forceSmeReceipt");
    if (sme.isForceGsmDataCoding())
      addFlag(flags, "forceGsmDataCoding");
    if (sme.isSmppPlus())
      addFlag(flags, "smppPlus");
    return flags.toString();
  }

  private static void storeSme(String smeId, Sme sme, PrintWriter out) {

    out.println("  <smerecord type=\"smpp\" uid=\"" + smeId + "\">");

    out.println("    <param name=\"priority\"          value=\"" + sme.getPriority() + "\"/>");

    out.println("    <param name=\"systemType\"        value=\"" + StringEncoderDecoder.encode(sme.getSystemType()) + "\"/>");
    out.println("    <param name=\"password\"          value=\"" + StringEncoderDecoder.encode(sme.getPassword() == null ? "" : sme.getPassword()) + "\"/>");
    out.println("    <param name=\"addrRange\"         value=\"" + StringEncoderDecoder.encode(sme.getAddrRange() == null ? "" : sme.getAddrRange()) + "\"/>");
    out.println("    <param name=\"smeN\"              value=\"" + sme.getSmeN() + "\"/>");
    out.println("    <param name=\"wantAlias\"         value=\"" + (sme.isWantAlias() ? "yes" : "no") + "\"/>");
    out.println("    <param name=\"timeout\"           value=\"" + sme.getTimeout() + "\"/>");
    out.println("    <param name=\"receiptSchemeName\" value=\"" + StringEncoderDecoder.encode(sme.getReceiptSchemeName()) + "\"/>");
    out.println("    <param name=\"disabled\"          value=\"" + sme.isDisabled() + "\"/>");


    String mode = null;
    for (Map.Entry<String, SmeBindMode> e : SME_BIND_MODES.entrySet()) {
      if (e.getValue() == sme.getBindMode()) {
        mode = e.getKey();
        break;
      }
    }
    out.println("    <param name=\"mode\"              value=\"" + mode + "\"/>");
    out.println("    <param name=\"proclimit\"         value=\"" + sme.getProclimit() + "\"/>");
    out.println("    <param name=\"schedlimit\"        value=\"" + sme.getSchedlimit() + "\"/>");
    out.println("    <param name=\"accessMask\"        value=\"" + sme.getAccessMask() + "\"/>");
    out.println("    <param name=\"flags\"              value=\"" + getFlagsStr(sme) + "\"/>");
    out.println("  </smerecord>");
  }

  



  public void save(InputStream oldFile, OutputStream newFile, Map<String, Sme> smes) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(newFile));
      XmlUtils.storeConfigHeader(out, "records", "SmeRecords.dtd");
      for (Map.Entry<String, Sme> e : smes.entrySet())
        storeSme(e.getKey(), e.getValue(), out);
      XmlUtils.storeConfigFooter(out, "records");
    } finally {
      if (out != null)
        out.close();
    }
  }

  private static void readSme(Map<String, Sme> smes, Element smeElement) throws AdminException {
    String smeId = smeElement.getAttribute("uid").trim();

    Sme sme = new Sme();

    NodeList list = smeElement.getElementsByTagName("param");
    for (int i = 0; i < list.getLength(); i++) {
      Element paramElem = (Element) list.item(i);
      String name = paramElem.getAttribute("name");
      String value = paramElem.getAttribute("value");
      if (name.equals("priority")) {
        sme.setPriority(Integer.parseInt(value));
      } else if (name.equals("interfaceVersion")) {
        int interfaceVersion = Integer.parseInt(value);
        setInterfaceVersion(sme, interfaceVersion);
      } else if (name.equals("systemType")) {
        sme.setSystemType(value);
      } else if (name.equals("password")) {
        sme.setPassword(value);
      } else if (name.equals("addrRange")) {
        sme.setAddrRange(value);
      } else if (name.equals("smeN")) {
        sme.setSmeN(Integer.parseInt(value));
      } else if (name.equals("wantAlias")) {
        sme.setWantAlias(value.equalsIgnoreCase("yes") || value.equalsIgnoreCase("true"));
      } else if (name.equals("forceDC")) {
        sme.setForceGsmDataCoding(Boolean.parseBoolean(value));
      } else if (name.equals("timeout")) {
        sme.setTimeout(Integer.parseInt(value));
      } else if (name.equals("receiptSchemeName")) {
        sme.setReceiptSchemeName(value); // TODO DEPENDENCY
      } else if (name.equals("disabled")) {
        sme.setDisabled(Boolean.parseBoolean(value));
      } else if (name.equals("mode")) {
        sme.setBindMode(SME_BIND_MODES.get(value));
      } else if (name.equals("proclimit")) {
        sme.setProclimit(Integer.parseInt(value));
      } else if (name.equals("schedlimit")) {
        sme.setSchedlimit(Integer.parseInt(value));
      } else if (name.equals("accessMask")) {
        sme.setAccessMask(Integer.parseInt(value));
      } else if (name.equals("flags")) {
        StringTokenizer flags = new StringTokenizer(value, ",");
        while (flags.hasMoreTokens()) {
          String flag = flags.nextToken().trim();
          if (flag.equals("carryOrgDescriptor"))
            sme.setCarryOrgDescriptor(true);
          else if (flag.equals("carryOrgUserInfo"))
            sme.setCarryOrgUserInfo(true);
          else if (flag.equals("carrySccpInfo"))
            sme.setCarrySccpInfo(true);
          else if (flag.equals("fillExtraDescriptor"))
            sme.setFillExtraDescriptor(true);
          else if (flag.equals("forceSmeReceipt"))
            sme.setForceSmeReceipt(true);
          else if (flag.equals("forceGsmDataCoding"))
            sme.setForceGsmDataCoding(true);
          else if (flag.equals("smppPlus"))
            sme.setSmppPlus(true);
        }
      }
    }
    smes.put(smeId, sme);
  }

  public Map<String, Sme> load(InputStream is) throws Exception {
    Map<String, Sme> smes = new HashMap<String, Sme>();
    Document smesDoc = XmlUtils.parse(is);
    NodeList smerecords = smesDoc.getElementsByTagName("smerecord");
    for (int j = 0; j < smerecords.getLength(); j++) {
      Element smeElement = (Element) smerecords.item(j);
      readSme(smes, smeElement);
    }

    return smes;
  }

  private static void setInterfaceVersion(Sme sme, int interfaceVersion) {
    if ((interfaceVersion >> 4) == 5)
      sme.setForceSmeReceipt(true);
    if ((interfaceVersion & 0xF) == 9) {
      sme.setCarryOrgDescriptor(true);
      sme.setCarryOrgUserInfo(true);
      sme.setCarrySccpInfo(true);
      sme.setFillExtraDescriptor(true);
    }
  }
}

