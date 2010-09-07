package ru.novosoft.smsc.admin.closed_groups;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ConfigHelper;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * todo сделать более умную проверку на isBroken. Если configBroken == true, то проверить статус во всех центрах (вдруг уже починили)
 * @author Artem Snopkov
 */
public class ClosedGroupManager implements SmscConfiguration {

  protected final static String PARAM_NAME_last_used_id = "lastId";
  protected final static String SECTION_NAME_group = "group";
  protected final static String ROOT_ELEMENT = "records";
  private final static int CLOSED_GROUP_NAME_MAXLENGTH = 64;

  private final ValidationHelper vh = new ValidationHelper(ClosedGroup.class.getCanonicalName());

  private final File configFile;
  private final File backupDir;
  private final ClusterController cc;
  private final FileSystem fs;
  protected Collection<ClosedGroup> groups;

  private boolean configBroken;
  private AtomicInteger lastGroupId;

  public ClosedGroupManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.cc = cc;
    this.fs = fs;
    load(); // todo make lazy load
  }

  public boolean isBroken() {
    return configBroken;
  }

  void checkBroken() throws AdminException {
    if (configBroken)
      throw new ClosedGroupException("config_broken");
  }

  public Collection<ClosedGroup> groups() throws AdminException {
    checkBroken();
    return new ArrayList<ClosedGroup>(groups);
  }

  long getLastGroupId() {
    return lastGroupId.longValue();
  }

  protected int getNextId() {
    return lastGroupId.incrementAndGet();
  }

  public ClosedGroup addGroup(String name, String description) throws AdminException {
    vh.checkLen("name", name, 1, CLOSED_GROUP_NAME_MAXLENGTH);

    checkBroken();

    int groupId = getNextId();
    ClosedGroup newGroup = new ClosedGroup(groupId, name, description, new ArrayList<Address>(), cc, this);

    if (cc.isOnline())
      cc.addClosedGroup(groupId, name);

    groups.add(newGroup);
    try {
      save();
    } catch (AdminException e) {
      groups.remove(newGroup);
      throw e;
    }

    return newGroup;
  }

  public boolean removeGroup(int groupId) throws AdminException {
    checkBroken();

    ClosedGroup group2remove = null;
    for (ClosedGroup g : groups) {
      if (g.getId() == groupId) {
        group2remove = g;
        break;
      }
    }

    if (group2remove == null)
      return false;

    //todo Проверить, что группа не используется ни в одном профиле при помощи вызова команды CgmListAbonents. Она возвращает список абонентов, которые могут
    //todo отправлять сообщения в данную группу

    if (cc.isOnline())
      cc.removeClosedGroup(groupId);

    groups.remove(group2remove);
    try {
      save();
    } catch (AdminException e) {
      groups.add(group2remove);
      throw e;
    }

    return true;
  }

  private ClosedGroup parseClosedGroup(Element closedGroupElem) {
    int id = Integer.parseInt(closedGroupElem.getAttribute("id"));
    String name;
    String elemName = closedGroupElem.getAttribute("name");
    if (elemName.length() > 64)
      name = elemName.substring(0, 64);
    else
      name = elemName;

    String def = closedGroupElem.getAttribute("desc");
    NodeList masksList = closedGroupElem.getElementsByTagName("mask");
    NodeList addrList = closedGroupElem.getElementsByTagName("address");
    Collection<Address> masks = new ArrayList<Address>(masksList.getLength() + addrList.getLength());
    for (int j = 0; j < masksList.getLength(); j++) {
      Element maskElem = (Element) masksList.item(j);
      masks.add(new Address(maskElem.getAttribute("value").trim()));
    }
    for (int j = 0; j < addrList.getLength(); j++) {
      Element maskElem = (Element) addrList.item(j);
      masks.add(new Address(maskElem.getAttribute("value").trim()));
    }

    return new ClosedGroup(id, name, def, masks, cc, this);
  }

  protected void load(InputStream is) throws AdminException {
    Collection<ClosedGroup> newGroups = new ArrayList<ClosedGroup>();
    try {
      if (cc.isOnline())
        cc.lockClosedGroups(false);
      Document closedGroupDoc = XmlUtils.parse(is);
      NodeList a = closedGroupDoc.getElementsByTagName(PARAM_NAME_last_used_id);
      if (a.getLength() > 0)
        lastGroupId = new AtomicInteger(Integer.parseInt(((Element) a.item(0)).getAttribute("value")));

      NodeList closedGroupsList = closedGroupDoc.getDocumentElement().getElementsByTagName(ClosedGroupManager.SECTION_NAME_group);
      for (int i = 0; i < closedGroupsList.getLength(); i++) {
        Element closedGroupElem = (Element) closedGroupsList.item(i);
        newGroups.add(parseClosedGroup(closedGroupElem));
      }

    } catch (Exception e) {
      configBroken = true;
      throw new ClosedGroupException("config_broken", e);
    } finally {
      if (cc.isOnline())
        cc.unlockClosedGroups();
    }

    groups = newGroups;
  }

  protected void load() throws AdminException {
    load(fs.getInputStream(configFile));
  }

  void save() throws AdminException {
    ConfigHelper.createBackup(configFile, backupDir, fs);

    File tmpConfigFile = new File(configFile.getAbsolutePath() + ".tmp");

    PrintWriter out = null;
    Throwable configBrokenCause = null;
    try {
      if (cc.isOnline())
        cc.lockClosedGroups(true);

      out = new PrintWriter(new OutputStreamWriter(fs.getOutputStream(tmpConfigFile), Functions.getLocaleEncoding()));
      XmlUtils.storeConfigHeader(out, ROOT_ELEMENT, "ClosedGroups.dtd", Functions.getLocaleEncoding());
      out.println("   <" + PARAM_NAME_last_used_id + " value=\"" + lastGroupId.longValue() + "\"/>");

      for (ClosedGroup g : groups) {
        out.print("   <group id=\"");
        out.print(g.getId());
        out.print("\" name=\"");
        out.print(StringEncoderDecoder.encode(g.getName()));
        out.print("\" desc=\"");
        out.print(StringEncoderDecoder.encode(g.getDescription()));
        out.println("\">");
        for (Address mask : g.getMasks()) {
          out.print("       <mask value=\"");
          out.print(StringEncoderDecoder.encode(mask.getNormalizedAddress()));
          out.println("\"/>");
        }
        out.println("   </group>");
      }

      XmlUtils.storeConfigFooter(out, ROOT_ELEMENT);
    } catch (Exception e) {
      configBroken = true;
      configBrokenCause = e;
    } finally {
      if (out != null)
        out.close();
      try {
        if (cc.isOnline())
          cc.unlockClosedGroups();
      } catch (AdminException e) {
        configBroken = true;
        configBrokenCause = e;
      }
    }

    if (configBroken) {
      fs.delete(tmpConfigFile);
      throw new ClosedGroupException("config_broken", configBrokenCause);
    }

    try {
      fs.delete(configFile);
      fs.rename(tmpConfigFile, configFile);
    } catch (AdminException e) {
      configBroken = true;
      throw new ClosedGroupException("config_broken", e);
    }
  }


  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState configState = cc.getClosedGroupConfigState();

    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();

    long ccLastUpdateTime = configState.getCcLastUpdateTime();
    for (Map.Entry<Integer, Long> e : configState.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = (e.getValue() >= ccLastUpdateTime) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }

    return result;
  }
}
