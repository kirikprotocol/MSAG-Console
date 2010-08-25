package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.beans.CommonConfig;
import ru.novosoft.smsc.web.beans.InstanceConfig;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.AppliableConfiguration;

import javax.faces.application.FacesMessage;
import java.security.Principal;
import java.text.MessageFormat;
import java.util.*;

/**
 * author: alkhal
 */
public class SmscConfigController extends SmscController{

  private static final Logger logger = Logger.getLogger(SmscConfigController.class);

  private AppliableConfiguration conf;

  private int instancesCount = 0;

  private CommonConfig commonConfig = new CommonConfig();

  private List<InstanceConfig> instanceConfigs = new LinkedList<InstanceConfig>();

  private boolean initialized = false;

  private DynamicTableModel addSsnModel = new DynamicTableModel();

  private DynamicTableModel localesModel = new DynamicTableModel();

  private DynamicTableModel directivesModel = new DynamicTableModel();

  private long lastUpdate;


  public SmscConfigController() {
    Map<String, String> reguestMap = getRequestParameters();
    conf = WebContext.getInstance().getAppliableConfiguration();
    SmscSettings smscSettings = conf.getSmscSettings();
    if(!reguestMap.containsKey("initialized")) {
      lastUpdate = conf.getSmscSettigsUpdateInfo().getLastUpdateTime();
      instancesCount = smscSettings.getSmscInstancesCount();        
      initCommonConfig();
      initInstances();
    }
    try{
      List<Integer> outOfDate = new LinkedList<Integer>();
      for(Map.Entry<Integer, SmscConfigurationStatus> e : conf.getSmscSettingsStatus().entrySet()) {
        if(e.getValue() == SmscConfigurationStatus.OUT_OF_DATE) {
          outOfDate.add(e.getKey());
        }
      }
      if(!outOfDate.isEmpty()) {
        String message = MessageFormat.format(
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString("smsc.config.instance.out_of_date"),
            outOfDate.toString());
        addMessage(FacesMessage.SEVERITY_WARN, message);
      }
    }catch (AdminException e) {
      logger.error(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
    }
  }

  private void initCommonConfig() {
    CommonSettings cs = conf.getSmscSettings().getCommonSettings();
    //core
    commonConfig.setState_machines_count(cs.getState_machines_count());
    commonConfig.setMainLoopsCount(cs.getMainLoopsCount());
    commonConfig.setEventQueueLimit(cs.getEventQueueLimit());
    commonConfig.setService_center_address(cs.getService_center_address());
    commonConfig.setUssd_center_address(cs.getUssd_center_address());
    commonConfig.setUssd_ssn(cs.getUssd_ssn());
    for(String ssn : cs.getAdd_ussd_ssn()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("ssn", ssn);
      addSsnModel.addRow(row);
    }
    commonConfig.setSystemId(cs.getSystemId());
    commonConfig.setService_type(cs.getService_type());
    commonConfig.setProtocol_id(cs.getProtocol_id());
    for(String l : cs.getLocales()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("locale", l);
      localesModel.addRow(row);
    }
    commonConfig.setDefault_locale(cs.getDefault_locale());
    commonConfig.setMergeTimeout(cs.getMergeTimeout());
    commonConfig.setTimezones_config(cs.getTimezones_config());
    commonConfig.setSmartMultipartForward(cs.isSmartMultipartForward());
    commonConfig.setSrcSmeSeparateRouting(cs.isSrcSmeSeparateRouting());
    commonConfig.setSchedulerSoftLimit(cs.getSchedulerSoftLimit());
    commonConfig.setSchedulerHardLimit(cs.getSchedulerHardLimit());

    //sms

    commonConfig.setSmsMaxValidTime(cs.getSmsMaxValidTime());

    // map

    commonConfig.setMapBusyMTDelay(cs.getMapBusyMTDelay());
    commonConfig.setMapLockedByMODelay(cs.getMapLockedByMODelay());
    commonConfig.setMapMOLockTimeout(cs.getMapMOLockTimeout());
    commonConfig.setMapAllowCallBarred(cs.isMapAllowCallBarred());
    commonConfig.setMapUssdV1Enabled(cs.isMapUssdV1Enabled());
    commonConfig.setMapUssdV1UseOrigEntityNumber(cs.isMapUssdV1UseOrigEntityNumber());
    commonConfig.setMapIOTasksCount(cs.getMapIOTasksCount());

    // trafficControl

    commonConfig.setTrafficShapeTimeFrame(cs.getTrafficShapeTimeFrame());
    commonConfig.setTrafficStatTimeFrame(cs.getTrafficStatTimeFrame());

    // MessageStore

    commonConfig.setMsArchiveInterval(cs.getMsArchiveInterval());

    // MessageStore.LocalStore

    commonConfig.setLsMaxStoreSize(cs.getLsMaxStoreSize());
    commonConfig.setLsMinRollTime(cs.getLsMinRollTime());

    // smpp

    commonConfig.setSmppReadTimeout(cs.getSmppReadTimeout());
    commonConfig.setSmppInactivityTime(cs.getSmppInactivityTime());
    commonConfig.setSmppInactivityTimeOut(cs.getSmppInactivityTimeOut());
    commonConfig.setSmppBindTimeout(cs.getSmppBindTimeout());
    commonConfig.setSmppDefaultConnectionsLimit(cs.getSmppDefaultConnectionsLimit());

    // profiler

    commonConfig.setProfilerSystemId(cs.getProfilerSystemId());
    commonConfig.setProfilerServiceType(cs.getProfilerServiceType());
    commonConfig.setProfilerProtocolId(cs.getProfilerProtocolId());
    commonConfig.setProfilerStoreFile(cs.getProfilerStoreFile());

    //profiler.ussdOpsMapping

    commonConfig.setProfilerReportNone(cs.getProfilerReportNone());
    commonConfig.setProfilerReportFull(cs.getProfilerReportFull());
    commonConfig.setProfilerReportFinal(cs.getProfilerReportFinal());
    commonConfig.setProfilerLocaleRU(cs.getProfilerLocaleRU());
    commonConfig.setProfilerLocaleEN(cs.getProfilerLocaleEN());
    commonConfig.setProfilerDefault(cs.getProfilerDefault());
    commonConfig.setProfilerUCS2(cs.getProfilerUCS2());
    commonConfig.setProfilerHide(cs.getProfilerHide());
    commonConfig.setProfilerUnhide(cs.getProfilerUnhide());
    commonConfig.setProfilerUSSD7BitOn(cs.getProfilerUSSD7BitOn());
    commonConfig.setProfilerUSSD7BitOff(cs.getProfilerUSSD7BitOff());
    commonConfig.setProfilerDivertOn(cs.getProfilerDivertOn());
    commonConfig.setProfilerDivertOff(cs.getProfilerDivertOff());
    commonConfig.setProfilerDivertAbsentOn(cs.getProfilerDivertAbsentOn());
    commonConfig.setProfilerDivertAbsentOff(cs.getProfilerDivertAbsentOff());
    commonConfig.setProfilerDivertBlockedOn(cs.getProfilerDivertBlockedOn());
    commonConfig.setProfilerDivertBlockedOff(cs.getProfilerDivertBlockedOff());
    commonConfig.setProfilerDivertBarredOn(cs.getProfilerDivertBarredOn());
    commonConfig.setProfilerDivertBarredOff(cs.getProfilerDivertBarredOff());
    commonConfig.setProfilerDivertCapacityOn(cs.getProfilerDivertCapacityOn());
    commonConfig.setProfilerDivertCapacityOff(cs.getProfilerDivertCapacityOff());
    commonConfig.setProfilerDivertStatus(cs.getProfilerDivertStatus());
    commonConfig.setProfilerDivertTo(cs.getProfilerDivertTo());
    commonConfig.setProfilerConcatOn(cs.getProfilerConcatOn());
    commonConfig.setProfilerConcatOff(cs.getProfilerConcatOff());
    commonConfig.setProfilerTranslitOn(cs.getProfilerTranslitOn());
    commonConfig.setProfilerTranslitOff(cs.getProfilerTranslitOff());

    // abonentinfo

    commonConfig.setAbInfoSystemId(cs.getAbInfoSystemId());
    commonConfig.setAbInfoServiceType(cs.getAbInfoServiceType());
    commonConfig.setAbInfoProtocolId(cs.getAbInfoProtocolId());
    commonConfig.setAbInfoMobileAccessAddress(cs.getAbInfoMobileAccessAddress());
    commonConfig.setAbInfoSmppAccessAddress(cs.getAbInfoSmppAccessAddress());

    // MscManager

    commonConfig.setMscAutomaticRegistration(cs.isMscAutomaticRegistration());
    commonConfig.setMscFailureLimit(cs.getMscFailureLimit());
    commonConfig.setMscStoreFile(cs.getMscStoreFile());
    commonConfig.setMscSingleAttemptTimeout(cs.getMscSingleAttemptTimeout());

    // acl

    commonConfig.setAclStoreDir(cs.getAclStoreDir());
    commonConfig.setAclPreCreateSize(cs.getAclPreCreateSize());

    // inman

    commonConfig.setInmanHost(cs.getInmanHost());
    commonConfig.setInmanPort(cs.getInmanPort());

    // inman.chargingPolicy

    commonConfig.setInmanChargingPeer2peer(cs.getInmanChargingPeer2peer());
    commonConfig.setInmanChargingOther(cs.getInmanChargingOther());

    // aliasman

    commonConfig.setAliasStoreFile(cs.getAliasStoreFile());

    // snmp

    commonConfig.setSnmpCsvFileDir(cs.getSnmpCsvFileDir());
    commonConfig.setSnmpCsvFileRollInterval(cs.getSnmpCsvFileRollInterval());
    commonConfig.setSnmpCacheTimeout(cs.getSnmpCacheTimeout());

    // directives

    for(Map.Entry<String, String> e : cs.getDirectives().entrySet()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("key", e.getKey());
      row.setValue("value", e.getValue());
      directivesModel.addRow(row);
    }

  }

  private void initInstances() {
    for (int i = 0; i < instancesCount; i++) {
      InstanceSettings is = conf.getSmscSettings().getInstanceSettings(i);
      InstanceConfig instnceConfig = new InstanceConfig();
      instanceConfigs.add(instnceConfig);
      instnceConfig.setInstanceNumber(i);

      //core
      instnceConfig.setAdminHost(is.getAdminHost());
      instnceConfig.setAdminPort(is.getAgentPort());

      //core.performance
      instnceConfig.setCorePerfHost(is.getCorePerfHost());
      instnceConfig.setCorePerfPort(is.getCorePerfPort());

      //core.smeperformance
      instnceConfig.setSmePerfHost(is.getSmePerfHost());
      instnceConfig.setSmePerfPort(is.getSmePerfPort());

      //MessageStore
      instnceConfig.setMessageStoreArchDir(is.getMessageStoreArchDir());
      instnceConfig.setMessageStoreStatsDir(is.getMessageStoreStatsDir());

      //MessageStore.LocalStore
      instnceConfig.setLocalStoreFilename(is.getLocalStoreFilename());
      instnceConfig.setLocalStoreMessIdFile(is.getLocalStoreMessIdFile());

      //smpp
      instnceConfig.setSmppHost(is.getSmppHost());
      instnceConfig.setSmppPort(is.getSmppPort());

      //profiler.notify
      instnceConfig.setAgentHost(is.getAgentHost());
      instnceConfig.setAgentPort(is.getAgentPort());
      instnceConfig.setCacheDir(is.getCacheDir());
    }
  }

  public DynamicTableModel getAddSsnModel() {
    return addSsnModel;
  }

  public void setAddSsnModel(DynamicTableModel addSsnModel) {
    this.addSsnModel = addSsnModel;
  }

  public DynamicTableModel getLocalesModel() {
    return localesModel;
  }

  public void setLocalesModel(DynamicTableModel localesModel) {
    this.localesModel = localesModel;
  }

  public DynamicTableModel getDirectivesModel() {
    return directivesModel;
  }

  public void setDirectivesModel(DynamicTableModel directivesModel) {
    this.directivesModel = directivesModel;
  }

  public int getInstancesCount() {
    return instancesCount;
  }


  public List<InstanceConfig> getInstanceConfigs() {
    return instanceConfigs;
  }

  public CommonConfig getCommonConfig() {
    return commonConfig;
  }


  public void setCommonConfig(CommonConfig commonConfig) {
    this.commonConfig = commonConfig;
  }

  public void setInstanceConfigs(List<InstanceConfig> instanceConfigs) {
    this.instanceConfigs = instanceConfigs;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public void setInstancesCount(int instancesCount) {
    this.instancesCount = instancesCount;
  }

  public long getLastUpdate() {
    return lastUpdate;
  }

  public void setLastUpdate(long lastUpdate) {
    this.lastUpdate = lastUpdate;
  }

  public String save() {

    if(lastUpdate != conf.getSmscSettigsUpdateInfo().getLastUpdateTime()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    for(DynamicTableRow row : addSsnModel.getRows()) {
      String value = (String)row.getValue("ssn");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "add_ssn.empty");
        return null;
      }
    }
    for(DynamicTableRow row : localesModel.getRows()) {
      String value = (String)row.getValue("locale");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "locale.empty");
        return null;
      }
    }
    for(DynamicTableRow row : directivesModel.getRows()) {
      String value = (String)row.getValue("key");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "directives.empty");   
        return null;        
      }
    }

    Locale locale = getLocale();
    try{
      CommonSettings cs = convert(commonConfig);
      InstanceSettings[] iss = new InstanceSettings[instancesCount];
      for(int i=0;i<instancesCount;i++) {
        iss[i] = convert(instanceConfigs.get(i), i);
      }

      SmscSettings smscSettings = conf.getSmscSettings();

      smscSettings.setCommonSettings(cs);
      for(int i=0;i<instancesCount;i++) {
        smscSettings.setInstanceSettings(i, iss[i]);
      }

      Principal p = getUserPrincipal();

      conf.setSmscSettings(smscSettings, p.getName());

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(locale));
      return null;
    }

  }

  private InstanceSettings convert(InstanceConfig instanceConfig, int instanceNumb) throws AdminException {
    InstanceSettings instanceSettings = conf.getSmscSettings().getInstanceSettings(instanceNumb);

    //core
    instanceSettings.setAdminHost(instanceConfig.getAdminHost());
    instanceSettings.setAdminPort(instanceConfig.getAgentPort());

    //core.performance
    instanceSettings.setCorePerfHost(instanceConfig.getCorePerfHost());
    instanceSettings.setCorePerfPort(instanceConfig.getCorePerfPort());

    //core.smeperformance
    instanceSettings.setSmePerfHost(instanceConfig.getSmePerfHost());
    instanceSettings.setSmePerfPort(instanceConfig.getSmppPort());

    //MessageStore
    instanceSettings.setMessageStoreArchDir(instanceConfig.getMessageStoreArchDir());
    instanceSettings.setMessageStoreStatsDir(instanceConfig.getMessageStoreStatsDir());

    //MessageStore.LocalStore
    instanceSettings.setLocalStoreFilename(instanceConfig.getLocalStoreFilename());
    instanceSettings.setLocalStoreMessIdFile(instanceConfig.getLocalStoreMessIdFile());

    //smpp
    instanceSettings.setSmppHost(instanceConfig.getSmppHost());
    instanceSettings.setSmppPort(instanceConfig.getSmppPort());

    //profiler.notify
    instanceSettings.setAgentHost(instanceConfig.getAgentHost());
    instanceSettings.setAgentPort(instanceConfig.getAgentPort());
    instanceSettings.setCacheDir(instanceConfig.getCacheDir());
    return instanceSettings;
  }

  private CommonSettings convert(CommonConfig cc) throws AdminException {
    CommonSettings commonSettings = conf.getSmscSettings().getCommonSettings();
//core
    commonSettings.setState_machines_count(cc.getState_machines_count());
    commonSettings.setMainLoopsCount(cc.getMainLoopsCount());
    commonSettings.setEventQueueLimit(cc.getEventQueueLimit());
    commonSettings.setService_center_address(cc.getService_center_address());
    commonSettings.setUssd_center_address(cc.getUssd_center_address());
    commonSettings.setUssd_ssn(cc.getUssd_ssn());
    String[] aus = new String[addSsnModel.getRowCount()];

    int i=0;
    for(DynamicTableRow s : addSsnModel.getRows()) {
      aus[i] = (String)s.getValue("ssn");
      i++;
    }
    commonSettings.setAdd_ussd_ssn(aus);

    commonSettings.setSystemId(cc.getSystemId());
    commonSettings.setService_type(cc.getService_type());
    commonSettings.setProtocol_id(cc.getProtocol_id());

    String[] ls = new String[localesModel.getRowCount()];
    i=0;
    for(DynamicTableRow s : localesModel.getRows()) {
      ls[i] = (String)s.getValue("locale");
      i++;
    }
    commonSettings.setLocales(ls);

    commonSettings.setDefault_locale(cc.getDefault_locale());
    commonSettings.setMergeTimeout(cc.getMergeTimeout());
    commonSettings.setTimezones_config(cc.getTimezones_config());
    commonSettings.setSmartMultipartForward(cc.isSmartMultipartForward());
    commonSettings.setSrcSmeSeparateRouting(cc.isSrcSmeSeparateRouting());
    commonSettings.setSchedulerSoftLimit(cc.getSchedulerSoftLimit());
    commonSettings.setSchedulerHardLimit(cc.getSchedulerHardLimit());

    //sms

    commonSettings.setSmsMaxValidTime(cc.getSmsMaxValidTime());

    // map

    commonSettings.setMapBusyMTDelay(cc.getMapBusyMTDelay());
    commonSettings.setMapLockedByMODelay(cc.getMapLockedByMODelay());
    commonSettings.setMapMOLockTimeout(cc.getMapMOLockTimeout());
    commonSettings.setMapAllowCallBarred(cc.isMapAllowCallBarred());
    commonSettings.setMapUssdV1Enabled(cc.isMapUssdV1Enabled());
    commonSettings.setMapUssdV1UseOrigEntityNumber(cc.isMapUssdV1UseOrigEntityNumber());
    commonSettings.setMapIOTasksCount(cc.getMapIOTasksCount());

    // trafficControl

    commonSettings.setTrafficShapeTimeFrame(cc.getTrafficShapeTimeFrame());
    commonSettings.setTrafficStatTimeFrame(cc.getTrafficStatTimeFrame());

    // MessageStore

    commonSettings.setMsArchiveInterval(cc.getMsArchiveInterval());

    // MessageStore.LocalStore

    commonSettings.setLsMaxStoreSize(cc.getLsMaxStoreSize());
    commonSettings.setLsMinRollTime(cc.getLsMinRollTime());

    // smpp

    commonSettings.setSmppReadTimeout(cc.getSmppReadTimeout());
    commonSettings.setSmppInactivityTime(cc.getSmppInactivityTime());
    commonSettings.setSmppInactivityTimeOut(cc.getSmppInactivityTimeOut());
    commonSettings.setSmppBindTimeout(cc.getSmppBindTimeout());
    commonSettings.setSmppDefaultConnectionsLimit(cc.getSmppDefaultConnectionsLimit());

    // profiler

    commonSettings.setProfilerSystemId(cc.getProfilerSystemId());
    commonSettings.setProfilerServiceType(cc.getProfilerServiceType());
    commonSettings.setProfilerProtocolId(cc.getProfilerProtocolId());
    commonSettings.setProfilerStoreFile(cc.getProfilerStoreFile());

    //profiler.ussdOpsMapping

    commonSettings.setProfilerReportNone(cc.getProfilerReportNone());
    commonSettings.setProfilerReportFull(cc.getProfilerReportFull());
    commonSettings.setProfilerReportFinal(cc.getProfilerReportFinal());
    commonSettings.setProfilerLocaleRU(cc.getProfilerLocaleRU());
    commonSettings.setProfilerLocaleEN(cc.getProfilerLocaleEN());
    commonSettings.setProfilerDefault(cc.getProfilerDefault());
    commonSettings.setProfilerUCS2(cc.getProfilerUCS2());
    commonSettings.setProfilerHide(cc.getProfilerHide());
    commonSettings.setProfilerUnhide(cc.getProfilerUnhide());
    commonSettings.setProfilerUSSD7BitOn(cc.getProfilerUSSD7BitOn());
    commonSettings.setProfilerUSSD7BitOff(cc.getProfilerUSSD7BitOff());
    commonSettings.setProfilerDivertOn(cc.getProfilerDivertOn());
    commonSettings.setProfilerDivertOff(cc.getProfilerDivertOff());
    commonSettings.setProfilerDivertAbsentOn(cc.getProfilerDivertAbsentOn());
    commonSettings.setProfilerDivertAbsentOff(cc.getProfilerDivertAbsentOff());
    commonSettings.setProfilerDivertBlockedOn(cc.getProfilerDivertBlockedOn());
    commonSettings.setProfilerDivertBlockedOff(cc.getProfilerDivertBlockedOff());
    commonSettings.setProfilerDivertBarredOn(cc.getProfilerDivertBarredOn());
    commonSettings.setProfilerDivertBarredOff(cc.getProfilerDivertBarredOff());
    commonSettings.setProfilerDivertCapacityOn(cc.getProfilerDivertCapacityOn());
    commonSettings.setProfilerDivertCapacityOff(cc.getProfilerDivertCapacityOff());
    commonSettings.setProfilerDivertStatus(cc.getProfilerDivertStatus());
    commonSettings.setProfilerDivertTo(cc.getProfilerDivertTo());
    commonSettings.setProfilerConcatOn(cc.getProfilerConcatOn());
    commonSettings.setProfilerConcatOff(cc.getProfilerConcatOff());
    commonSettings.setProfilerTranslitOn(cc.getProfilerTranslitOn());
    commonSettings.setProfilerTranslitOff(cc.getProfilerTranslitOff());

    // abonentinfo

    commonSettings.setAbInfoSystemId(cc.getAbInfoSystemId());
    commonSettings.setAbInfoServiceType(cc.getAbInfoServiceType());
    commonSettings.setAbInfoProtocolId(cc.getAbInfoProtocolId());
    commonSettings.setAbInfoMobileAccessAddress(cc.getAbInfoMobileAccessAddress());
    commonSettings.setAbInfoSmppAccessAddress(cc.getAbInfoSmppAccessAddress());

    // MscManager

    commonSettings.setMscAutomaticRegistration(cc.isMscAutomaticRegistration());
    commonSettings.setMscFailureLimit(cc.getMscFailureLimit());
    commonSettings.setMscStoreFile(cc.getMscStoreFile());
    commonSettings.setMscSingleAttemptTimeout(cc.getMscSingleAttemptTimeout());

    // acl

    commonSettings.setAclStoreDir(cc.getAclStoreDir());
    commonSettings.setAclPreCreateSize(cc.getAclPreCreateSize());

    // inman

    commonSettings.setInmanHost(cc.getInmanHost());
    commonSettings.setInmanPort(cc.getInmanPort());

    // inman.chargingPolicy

    commonSettings.setInmanChargingPeer2peer(cc.getInmanChargingPeer2peer());
    commonSettings.setInmanChargingOther(cc.getInmanChargingOther());

    // aliasman

    commonSettings.setAliasStoreFile(cc.getAliasStoreFile());

    // snmp

    commonSettings.setSnmpCsvFileDir(cc.getSnmpCsvFileDir());
    commonSettings.setSnmpCsvFileRollInterval(cc.getSnmpCsvFileRollInterval());
    commonSettings.setSnmpCacheTimeout(cc.getSnmpCacheTimeout());

    // directives

    Map<String,String> d = new HashMap<String, String>(directivesModel.getRowCount());
    for(DynamicTableRow e : directivesModel.getRows()) {
      d.put((String)e.getValue("key"), (String)e.getValue("value"));
    }
    commonSettings.setDirectives(d);

    return commonSettings;
  }
}
