package ru.novosoft.smsc.admin.fraud;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ConfigHelper;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.XmlUtils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.regex.Pattern;

/**
 * Класс для работы с конфигурацией Fraud
 * @author Artem Snopkov
 */
public class FraudManager {

  private static final Pattern mscAddressPattern = Pattern.compile("\\d{1,15}");

  private int tail;
  private boolean enableCheck;
  private boolean enableReject;
  private Collection<String> whiteList;
  private boolean changed;

  private File configFile;
  private File backupDir;
  private ClusterController cc;
  private FileSystem fs;

  public FraudManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.cc = cc;
    this.fs = fs;
    this.whiteList = new ArrayList<String>();
    reset();
  }

  public int getTail() {
    return tail;
  }

  public void setTail(int tail) {
    this.tail = tail;
    changed=true;
  }

  public boolean isEnableCheck() {
    return enableCheck;
  }

  public void setEnableCheck(boolean enableCheck) {
    this.enableCheck = enableCheck;
    changed=true;
  }

  public boolean isEnableReject() {
    return enableReject;
  }

  public void setEnableReject(boolean enableReject) {
    this.enableReject = enableReject;
    changed=true;
  }

  public Collection<String> getWhiteList() {
    return Collections.unmodifiableCollection(whiteList);
  }

  /**
   * Устанавливает список msc адресов.
   * @param whiteList новый список msc адресов.
   * @throws FraudException если новый список msc адресов содержит некорректные адреса.
   */
  public void setWhiteList(Collection<String> whiteList) throws FraudException {
    if (whiteList == null)
      throw new NullPointerException();
    for (String mscAddress : whiteList)
      if (!mscAddressPattern.matcher(mscAddress).matches())
        throw new FraudException("illegal_msc_address", mscAddress);
    this.whiteList = new ArrayList<String>(whiteList);
    changed=true;
  }

  /**
   * Возвращщает true, если конфигурация изменилась.
   * @return true, если конфигурация изменилась.
   */
  public boolean isChanged() {
    return changed;
  }

  protected void apply(OutputStream os) throws AdminException {
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

    } catch (IOException e) {
      throw new FraudException("write_config_error", e);
    } finally {
      if (out != null)
        out.close();
    }
  }

  /**
   * Сохраняет и применяет изменения, сделанные в fraud
   * @throws AdminException если сохранить или применить изменения не удалось
   */
  public synchronized void apply() throws AdminException {

    ConfigHelper.createBackup(configFile, backupDir, fs);

    File tmpFile = new File(configFile.getAbsolutePath() + ".tmp");

    apply(fs.getOutputStream(tmpFile));

    fs.delete(configFile);

    fs.rename(tmpFile, configFile);

    cc.applyFraud();

    changed=false;
  }

  protected void reset(InputStream is) throws IOException, SAXException, ParserConfigurationException {
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

    changed = false;
  }

  /**
   * Откатывает изменения, сделанные в конфигурации fraud
   * @throws AdminException если откатить изменения не удалось
   */
  public void reset() throws AdminException {

    InputStream is = null;
    try {
      is = fs.getInputStream(configFile);

      reset(is);

    } catch (Exception e) {
      throw new FraudException("read_config_error", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }
}
