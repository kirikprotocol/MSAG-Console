package ru.novosoft.smsc.web.controllers;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.application.FacesMessage;
import javax.faces.component.html.HtmlSelectOneMenu;
import javax.faces.event.ValueChangeEvent;
import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class SmscStatusController extends SmscController {

  private static final int COMPONENT_TYPE_SMSC=0;
  private static final int COMPONENT_TYPE_SME=1;
  private static final int COMPONENT_TYPE_OTHER=2;

  private final SmscManager smscManager;
  private final ClusterControllerManager ccManager;
  private final ArchiveDaemonManager archiveDaemonManager;
  private final SmeManager smeManager;
  private final Map<String, SmscConfiguration> configs;

  private Map<String, AbstractComponent> components;

  public SmscStatusController() {
    WebContext ctx = WebContext.getInstance();
    smscManager = ctx.getSmscManager();
    ccManager = ctx.getClusterControllerManager();
    archiveDaemonManager = ctx.getArchiveDaemonManager();
    smeManager = ctx.getSmeManager();
    configs = new HashMap<String, SmscConfiguration>();

    configs.put("acl", ctx.getAclManager());
    configs.put("alias", ctx.getAliasManager());
    configs.put("closed_group", ctx.getClosedGroupManager());
    configs.put("fraud", ctx.getFraudManager());
    configs.put("map_limit", ctx.getMapLimitManager());
    configs.put("msc", ctx.getMscManager());
    configs.put("profile", ctx.getProfileManager());
    configs.put("reschedule", ctx.getRescheduleManager());
    configs.put("resource", ctx.getResourceManager());
    configs.put("sme", ctx.getSmeManager());
    configs.put("smsc", ctx.getSmscManager());
    configs.put("snmp", ctx.getSnmpManager());
    configs.put("timezone", ctx.getTimezoneManager());
  }

  private synchronized Map<String, AbstractComponent> getComponents() throws AdminException {
    if (components != null)
      return components;

    Map<String, AbstractComponent> newComponents = new HashMap<String, AbstractComponent>();
    List<ConfigState> configStates = new ArrayList<ConfigState>();

    for (Map.Entry<String, SmscConfiguration> e : configs.entrySet()) {
      SmscConfiguration cfg = e.getValue();
      configStates.add(new ConfigState(e.getKey(), cfg));
    }

    SmscSettings smscSettings = smscManager.getSettings();
    for (int i = 0; i < smscSettings.getSmscInstancesCount(); i++) {
      String onlineHost = smscManager.getSmscOnlineHost(i);
      SmscComponent status = new SmscComponent(i, onlineHost, smscManager.getSmscHosts(i), configStates);
      newComponents.put(status.getName(), status);
    }

    ClusterControllerComponent ccComponent = new ClusterControllerComponent();
    newComponents.put(ccComponent.getName(), ccComponent);

    if (archiveDaemonManager != null) {
      ArchiveDaemonComponent adComponent = new ArchiveDaemonComponent();
      newComponents.put(adComponent.getName(), adComponent);
    }

    Collection<String> smeIds = smeManager.smes().keySet();
    for (String smeId : smeIds) {
      SmeServiceStatus smeStatus = smeManager.getSmeServiceStatus(smeId);
      if (smeStatus != null) {
        newComponents.put(smeId, new SmeComponent(smeId, smeStatus));
      }
    }

    this.components = newComponents;

    return components;
  }

  private void logError(AdminException e) {
    addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
  }

  private DataTableModel getComponentsByType(int type) {
    List<AbstractComponent> result = new ArrayList<AbstractComponent>();

    try {
      Map<String, AbstractComponent> components = getComponents();
      for (AbstractComponent c : components.values()) {
        if (c.type == type)
          result.add(c);
      }
    } catch (AdminException e) {
      addError(e);
    }

    return new ListTableModel(result);
  }

  public DataTableModel getSmsCenters() {
    return getComponentsByType(COMPONENT_TYPE_SMSC);
  }

  public DataTableModel getAuxiaryComponents() {
    return getComponentsByType(COMPONENT_TYPE_OTHER);
  }

  public DataTableModel getSmeComponents() {
    return getComponentsByType(COMPONENT_TYPE_SME);
  }

  public void startComponent() {
    String componentName = getRequestParameter("componentName");
    if (componentName != null) {
      try {
        AbstractComponent c = getComponents().get(componentName);
        if (c != null)
          c.start();
      } catch (AdminException e) {
        logError(e);
      }
    }
  }

  public void stopComponent() {
    String componentName = getRequestParameter("componentName");
    if (componentName != null) {
      try {
        AbstractComponent c = getComponents().get(componentName);
        if (c != null)
          c.stop();
      } catch (AdminException e) {
        logError(e);
      }
    }
  }

  public void switchComponent() {
    String componentName = getRequestParameter("componentName");
    String toHost = getRequestParameter("toHost");
    if (componentName != null && toHost != null) {
      try {
        AbstractComponent c = getComponents().get(componentName);
        if (c != null)
          c.switchTo(toHost);
      } catch (AdminException e) {
        logError(e);
      }
    }
  }

  /**
   *
   */
  public abstract class AbstractComponent implements Serializable {
    protected final String name;
    protected final String onlineHost;
    protected final List<String> hosts;
    protected int type;

    public AbstractComponent(int type, String name, String onlineHost, List<String> hosts) {
      this.type = type;
      this.name = name;
      this.onlineHost = onlineHost;
      this.hosts = hosts;
    }

    public String getName() {
      return name;
    }

    public String getOnlineHost() {
      return onlineHost;
    }

    public List<String> getHosts() {
      List<String> items = new ArrayList<String>(hosts.size());
      for (String host : hosts) {
        if (onlineHost == null || !host.equals(onlineHost))
          items.add(host);
      }
      return items;
    }

    public boolean isSwitchAllowed() {
      return hosts != null && hosts.size() > 1;
    }

    abstract void start() throws AdminException;
    abstract void stop() throws AdminException;
    abstract void switchTo(String toHost) throws AdminException;
  }

  /**
   *
   */
  public class SmscComponent extends AbstractComponent implements Serializable {

    private final int instanceNumber;
    private List<String> errors;

    public SmscComponent(int instanceNumber, String onlineHost, List<String> hosts, List<ConfigState> cfgStates) {
      super(COMPONENT_TYPE_SMSC, "SMSC " + (instanceNumber + 1), onlineHost, hosts);
      this.instanceNumber = instanceNumber;

      if (onlineHost != null) {
        errors = new ArrayList<String>();
        for (ConfigState cfgState : cfgStates) {
          if (cfgState.getStatusForSmsc(instanceNumber) == SmscConfigurationStatus.OUT_OF_DATE)
            errors.add("status.page." + cfgState.configName);
        }
      }
    }

    public int getInstanceNumber() {
      return instanceNumber;
    }

    public List<String> getErrors() {
      return errors;
    }

    public boolean isHasErrors() {
      return onlineHost != null && !errors.isEmpty();
    }

    public void start() throws AdminException {
      smscManager.startSmsc(instanceNumber);
    }
    public void stop() throws AdminException {
      smscManager.stopSmsc(instanceNumber);
    }
    public void switchTo(String toHost) throws AdminException {
      smscManager.switchSmsc(instanceNumber, toHost);
    }
  }

  /**
   *
   */
  public class ClusterControllerComponent extends AbstractComponent implements Serializable {
    public ClusterControllerComponent() throws AdminException {
      super(COMPONENT_TYPE_OTHER, "ClusterController", ccManager.getControllerOnlineHost(), ccManager.getControllerHosts());
    }
    public void start() throws AdminException {
      ccManager.startClusterController();
    }
    public void stop() throws AdminException {
      ccManager.stopClusterController();
    }
    public void switchTo(String toHost) throws AdminException {
      ccManager.switchClusterController(toHost);
    }
  }

  /**
   *
   */
  public class ArchiveDaemonComponent extends AbstractComponent implements Serializable {
    public ArchiveDaemonComponent() throws AdminException {
      super(COMPONENT_TYPE_OTHER, "ArchiveDaemon", archiveDaemonManager.getDaemonOnlineHost(), archiveDaemonManager.getDaemonHosts());
    }
    public void start() throws AdminException {
      archiveDaemonManager.startDaemon();
    }
    public void stop() throws AdminException {
      archiveDaemonManager.stopDaemon();
    }
    public void switchTo(String toHost) throws AdminException {
      archiveDaemonManager.switchDaemon(toHost);
    }
  }

  /**
   *
   */
  public class SmeComponent extends AbstractComponent implements Serializable {
    public SmeComponent(String smeId, SmeServiceStatus status) throws AdminException {
      super(COMPONENT_TYPE_SME, smeId, status.getOnlineHost(), status.getHosts());
    }
    public void start() throws AdminException {
      smeManager.startSme(name);
    }
    public void stop() throws AdminException {
      smeManager.stopSme(name);
    }
    public void switchTo(String toHost) throws AdminException {
      smeManager.switchSme(name, toHost);
    }
  }

  /**
   *
   */
  private class ConfigState {
    private final String configName;
    private final Map<Integer, SmscConfigurationStatus> statuses;

    ConfigState(String configName, SmscConfiguration cfg) throws AdminException {
      this.configName = configName;
      this.statuses = cfg.getStatusForSmscs();
    }

    public SmscConfigurationStatus getStatusForSmsc(int instanceNumber) {
      return statuses.get(instanceNumber);
    }
  }

  /**
   *
   */
  public class ListTableModel implements DataTableModel {

    private final List values;

    public ListTableModel(List values) {
      this.values = values;
    }

    public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
      return values;
    }

    public int getRowsCount() {
      return values.size();
    }
  }

}
