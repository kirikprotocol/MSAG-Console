package ru.novosoft.smsc.admin.fraud;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 26.02.2008
 */

public class FraudConfigManager {
  private static final Category log = Category.getInstance(FraudConfigManager.class);
  private static FraudConfigManager instance = null;

  private final File configFile;
  private final Set whiteMscAddresses;
  private int tail;
  private boolean enableCheck;
  private boolean enableReject;

  public static FraudConfigManager getInstance() throws AdminException {
    if (instance == null)
      instance = new FraudConfigManager();
    return instance;
  }

  private  FraudConfigManager() throws AdminException {
    try {
      configFile = new File(WebAppFolders.getSmscConfFolder(), "fraud.xml");
      whiteMscAddresses = new HashSet();
      load();
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't init fraud config manager. Reason: " + e.getMessage());
    }
  }

  public synchronized void addWhiteMscAddress(String mscAddress) throws AdminException {
    whiteMscAddresses.add(mscAddress);
  }

  public synchronized void removeWhiteMscAddress(String mscAddress) throws AdminException {
    whiteMscAddresses.remove(mscAddress);
  }

  public synchronized List getWhiteMscAddresses() {
    return new LinkedList(whiteMscAddresses);
  }

  private synchronized void load() throws AdminException {
    whiteMscAddresses.clear();
    try {
      Document fraudDoc = Utils.parse(configFile.getAbsolutePath());

      NodeList whiteLists = fraudDoc.getDocumentElement().getElementsByTagName("whitelist");

      if (whiteLists != null && whiteLists.getLength() > 0) {

        Element whiteList = (Element)whiteLists.item(0);

        NodeList mscs = whiteList.getElementsByTagName("msc");
        for (int i=0; i< mscs.getLength(); i++) {
          String msc = ((Element)mscs.item(i)).getAttribute("value");
          if (msc != null && msc.trim().length() > 0)
            whiteMscAddresses.add(msc.trim());
        }
      }

      NodeList policies = fraudDoc.getDocumentElement().getElementsByTagName("policy");

      if (policies != null && policies.getLength() > 0) {
        Element policy = (Element)policies.item(0);

        NodeList tails = policy.getElementsByTagName("tail");
        if (tails != null && tails.getLength() > 0) {
          String tailStr = ((Element)tails.item(0)).getAttribute("value");
          if (tailStr != null && tailStr.trim().length() > 0)
          this.tail = Integer.parseInt(tailStr);
        }

        NodeList checks = policy.getElementsByTagName("enableCheck");
        if (checks != null && checks.getLength() > 0) {
          String checkStr = ((Element)checks.item(0)).getAttribute("value");
          if (checkStr != null && checkStr.trim().length() > 0)
          this.enableCheck = Boolean.valueOf(checkStr).booleanValue();
        }

        NodeList rejects = policy.getElementsByTagName("enableReject");
        if (rejects != null && rejects.getLength() > 0) {
          String rejectStr = ((Element)rejects.item(0)).getAttribute("value");
          if (rejectStr != null && rejectStr.trim().length() > 0)
          this.enableReject = Boolean.valueOf(rejectStr).booleanValue();
        }
      }

    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't load fraud config. Reason: " + e.getMessage());
    }
  }

  private synchronized void save(File file) throws AdminException {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(file), Functions.getLocaleEncoding()));

      Functions.storeConfigHeader(out, "fraud", "fraud.dtd", Functions.getLocaleEncoding());

      out.println("  <policy>");
      out.println("    <tail value=\"" + tail + "\"/>");
      out.println("    <enableCheck value=\"" + enableCheck + "\"/>");
      out.println("    <enableReject value=\"" + enableReject + "\"/>");
      out.println("  </policy>");

      out.println("  <whitelist>");
      for (Iterator iter = whiteMscAddresses.iterator(); iter.hasNext();)
        out.println("    <msc value=\"" + iter.next() + "\"/>");
      out.println("  </whitelist>");

      Functions.storeConfigFooter(out, "fraud");

    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save fraud config. Reason: " + e.getMessage());
    } finally {
      if (out != null)
        out.close();
    }
  }

  public synchronized void save() throws AdminException {
    try {
      final File newFile = Functions.createNewFilenameForSave(configFile);
      save(newFile);
      Functions.renameNewSavedFileToOriginal(newFile, configFile);
    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save regions. Reason: " + e.getMessage());
    }
  }

  public synchronized void reset() throws AdminException {
    load();
  }

  public int getTail() {
    return tail;
  }

  public void setTail(int tail) {
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
}
