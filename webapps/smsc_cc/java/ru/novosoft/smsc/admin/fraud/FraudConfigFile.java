package ru.novosoft.smsc.admin.fraud;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.Functions;
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
class FraudConfigFile implements ManagedConfigFile<FraudSettings> {

  public void save(InputStream oldFile, OutputStream os, FraudSettings settings) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(os, Functions.getLocaleEncoding()));

      XmlUtils.storeConfigHeader(out, "fraud", "fraud.dtd", Functions.getLocaleEncoding());

      out.println("  <policy>");
      out.println("    <tail value=\"" + settings.getTail() + "\"/>");
      out.println("    <enableCheck value=\"" + settings.isEnableCheck() + "\"/>");
      out.println("    <enableReject value=\"" + settings.isEnableReject() + "\"/>");
      out.println("  </policy>");

      out.println("  <whitelist>");
      for (Object aWhiteList : settings.getWhiteList()) out.println("    <msc value=\"" + aWhiteList + "\"/>");
      out.println("  </whitelist>");

      XmlUtils.storeConfigFooter(out, "fraud");

    } finally {
      if (out != null)
        out.close();
    }
  }

  public FraudSettings load(InputStream is) throws Exception {
    Document fraudDoc = XmlUtils.parse(is);

    NodeList whiteLists = fraudDoc.getDocumentElement().getElementsByTagName("whitelist");

    FraudSettings s = new FraudSettings();

    List<Address> wl = new ArrayList<Address>();

    if (whiteLists != null && whiteLists.getLength() > 0) {

      Element whiteList = (Element) whiteLists.item(0);

      NodeList mscs = whiteList.getElementsByTagName("msc");
      for (int i = 0; i < mscs.getLength(); i++) {
        String msc = ((Element) mscs.item(i)).getAttribute("value");
        if (msc != null && msc.trim().length() > 0)
          wl.add(new Address(msc.trim()));
      }
    }

    s.setWhiteList(wl);

    NodeList policies = fraudDoc.getDocumentElement().getElementsByTagName("policy");

    if (policies != null && policies.getLength() > 0) {
      Element policy = (Element) policies.item(0);

      NodeList tails = policy.getElementsByTagName("tail");
      if (tails != null && tails.getLength() > 0) {
        String tailStr = ((Element) tails.item(0)).getAttribute("value");
        if (tailStr != null && tailStr.trim().length() > 0)
          s.setTail(Integer.parseInt(tailStr));
      }

      NodeList checks = policy.getElementsByTagName("enableCheck");
      if (checks != null && checks.getLength() > 0) {
        String checkStr = ((Element) checks.item(0)).getAttribute("value");
        if (checkStr != null && checkStr.trim().length() > 0)
          s.setEnableCheck(Boolean.valueOf(checkStr));
      }

      NodeList rejects = policy.getElementsByTagName("enableReject");
      if (rejects != null && rejects.getLength() > 0) {
        String rejectStr = ((Element) rejects.item(0)).getAttribute("value");
        if (rejectStr != null && rejectStr.trim().length() > 0)
          s.setEnableReject(Boolean.valueOf(rejectStr));
      }
    }

    return s;
  }
}
