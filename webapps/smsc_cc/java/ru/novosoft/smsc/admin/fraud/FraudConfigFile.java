package ru.novosoft.smsc.admin.fraud;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.regex.Pattern;

/**
 * @author Artem Snopkov
 */
class FraudConfigFile implements ManagedConfigFile {

  private static final Pattern mscAddressPattern = Pattern.compile("\\d{1,15}");

  private int tail;
  private boolean enableCheck;
  private boolean enableReject;
  private Collection<String> whiteList = new ArrayList<String>();

   public int getTail() {
    return tail;
  }

  public void setTail(int tail) {
    if (tail < 0)
      throw new IllegalArgumentException("tail");
    this.tail = tail;
  }

  public boolean isEnableCheck() {
    return enableCheck;
  }

  public void setEnableCheck(boolean enableCheck) {
    this.enableCheck = enableCheck;
  }

  public boolean isEnableReject() {
    return enableReject;
  }

  public void setEnableReject(boolean enableReject) {
    this.enableReject = enableReject;
  }

  public Collection<String> getWhiteList() {
    return Collections.unmodifiableCollection(whiteList);
  }

  /**
   * Устанавливает список msc адресов.
   * @param whiteList новый список msc адресов.
   */
  public void setWhiteList(Collection<String> whiteList) {
    if (whiteList == null)
      throw new NullPointerException();
    for (String mscAddress : whiteList)
      if (!mscAddressPattern.matcher(mscAddress).matches())
        throw new IllegalArgumentException("whiteList");
    this.whiteList = new ArrayList<String>(whiteList);
  }

  public void save(InputStream oldFile, OutputStream os) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(os, Functions.getLocaleEncoding()));

      XmlUtils.storeConfigHeader(out, "fraud", "fraud.dtd", Functions.getLocaleEncoding());

      out.println("  <policy>");
      out.println("    <tail value=\"" + tail + "\"/>");
      out.println("    <enableCheck value=\"" + enableCheck + "\"/>");
      out.println("    <enableReject value=\"" + enableReject + "\"/>");
      out.println("  </policy>");

      out.println("  <whitelist>");
      for (Iterator iter = whiteList.iterator(); iter.hasNext();)
        out.println("    <msc value=\"" + iter.next() + "\"/>");
      out.println("  </whitelist>");

      XmlUtils.storeConfigFooter(out, "fraud");

    } finally {
      if (out != null)
        out.close();
    }
  }

  public void load(InputStream is) throws Exception {
    Document fraudDoc = XmlUtils.parse(is);

    NodeList whiteLists = fraudDoc.getDocumentElement().getElementsByTagName("whitelist");

    this.whiteList.clear();

    if (whiteLists != null && whiteLists.getLength() > 0) {

      Element whiteList = (Element) whiteLists.item(0);

      NodeList mscs = whiteList.getElementsByTagName("msc");
      for (int i = 0; i < mscs.getLength(); i++) {
        String msc = ((Element) mscs.item(i)).getAttribute("value");
        if (msc != null && msc.trim().length() > 0)
          this.whiteList.add(msc.trim());
      }
    }

    NodeList policies = fraudDoc.getDocumentElement().getElementsByTagName("policy");

    if (policies != null && policies.getLength() > 0) {
      Element policy = (Element) policies.item(0);

      NodeList tails = policy.getElementsByTagName("tail");
      if (tails != null && tails.getLength() > 0) {
        String tailStr = ((Element) tails.item(0)).getAttribute("value");
        if (tailStr != null && tailStr.trim().length() > 0)
          this.tail = Integer.parseInt(tailStr);
      }

      NodeList checks = policy.getElementsByTagName("enableCheck");
      if (checks != null && checks.getLength() > 0) {
        String checkStr = ((Element) checks.item(0)).getAttribute("value");
        if (checkStr != null && checkStr.trim().length() > 0)
          this.enableCheck = Boolean.valueOf(checkStr).booleanValue();
      }

      NodeList rejects = policy.getElementsByTagName("enableReject");
      if (rejects != null && rejects.getLength() > 0) {
        String rejectStr = ((Element) rejects.item(0)).getAttribute("value");
        if (rejectStr != null && rejectStr.trim().length() > 0)
          this.enableReject = Boolean.valueOf(rejectStr).booleanValue();
      }
    }

  }
}
