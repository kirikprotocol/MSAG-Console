package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.util.HashMap;
import java.util.Map;

/**
 * Структура для хранения настроек, общих для всех СМСЦ
 *
 * @author Artem Snopkov
 */

public class CommonSettings implements Cloneable {

  // core

  private int state_machines_count;
  private int mainLoopsCount;
  private int eventQueueLimit;
  private String service_center_address;
  private String ussd_center_address;
  private int ussd_ssn;
  private String[] add_ussd_ssn;
  private String systemId;
  private String service_type;
  private int protocol_id;
  private String[] locales;
  private String default_locale;
  private int mergeTimeout;
  private String timezones_config;
  private boolean smartMultipartForward;
  private boolean srcSmeSeparateRouting;
  private int schedulerSoftLimit;
  private int schedulerHardLimit;

  // sms

  private int smsMaxValidTime;

  // map

  private int mapBusyMTDelay;
  private int mapLockedByMODelay;
  private int mapMOLockTimeout;
  private boolean mapAllowCallBarred;
  private boolean mapUssdV1Enabled;
  private boolean mapUssdV1UseOrigEntityNumber;
  private int mapIOTasksCount;

  // trafficControl

  private int trafficShapeTimeFrame;
  private int trafficStatTimeFrame;

  // MessageStore

  private int msArchiveInterval;

  // MessageStore.LocalStore

  private int lsMaxStoreSize;
  private int lsMinRollTime;

  // smpp

  private int smppReadTimeout;
  private int smppInactivityTime;
  private int smppInactivityTimeOut;
  private int smppBindTimeout;
  private int smppDefaultConnectionsLimit;

  // profiler

  private String profilerSystemId;
  private String profilerServiceType;
  private int profilerProtocolId;
  private String profilerStoreFile;

  //profiler.ussdOpsMapping

  private int profilerReportNone;
  private int profilerReportFull;
  private int profilerReportFinal;
  private int profilerLocaleRU;
  private int profilerLocaleEN;
  private int profilerDefault;
  private int profilerUCS2;
  private int profilerHide;
  private int profilerUnhide;
  private int profilerUSSD7BitOn;
  private int profilerUSSD7BitOff;
  private int profilerDivertOn;
  private int profilerDivertOff;
  private int profilerDivertAbsentOn;
  private int profilerDivertAbsentOff;
  private int profilerDivertBlockedOn;
  private int profilerDivertBlockedOff;
  private int profilerDivertBarredOn;
  private int profilerDivertBarredOff;
  private int profilerDivertCapacityOn;
  private int profilerDivertCapacityOff;
  private int profilerDivertStatus;
  private int profilerDivertTo;
  private int profilerConcatOn;
  private int profilerConcatOff;
  private int profilerTranslitOn;
  private int profilerTranslitOff;

  // abonentinfo

  private String abInfoSystemId;
  private String abInfoServiceType;
  private int abInfoProtocolId;
  private String abInfoMobileAccessAddress;
  private String abInfoSmppAccessAddress;

  // MscManager

  private boolean mscAutomaticRegistration;
  private int mscFailureLimit;
  private String mscStoreFile;
  private int mscSingleAttemptTimeout;

  // acl

  private String aclStoreDir;
  private int aclPreCreateSize;

  // inman

  private String inmanHost;
  private int inmanPort;

  // inman.chargingPolicy

  private String inmanChargingPeer2peer;
  private String inmanChargingOther;

  // aliasman

  private String aliasStoreFile;

  // snmp

  private String snmpCsvFileDir;
  private int snmpCsvFileRollInterval;
  private int snmpCacheTimeout;

  // directives

  private Map<String, String> directives = new HashMap<String, String>();

  private ValidationHelper vh = new ValidationHelper(CommonSettings.class.getCanonicalName());

  public CommonSettings() {

  }

  CommonSettings(XmlConfig c) throws XmlConfigException, AdminException {
    load(c);
  }

  protected void save(XmlConfig c) throws XmlConfigException {

    // core
    XmlConfigSection s = c.getOrCreateSection("core");
    s.setInt("state_machines_count", state_machines_count);
    s.setInt("mainLoopsCount", mainLoopsCount);
    s.setInt("eventQueueLimit", eventQueueLimit);
    s.setString("service_center_address", service_center_address);
    s.setString("ussd_center_address", ussd_center_address);
    s.setInt("ussd_ssn", ussd_ssn);
    s.setStringArray("add_ussd_ssn", add_ussd_ssn, ",");
    s.setString("systemId", systemId);
    s.setString("service_type", service_type);
    s.setInt("protocol_id", protocol_id);
    s.setStringArray("locales", locales, ",");
    s.setString("default_locale", default_locale);
    s.setInt("mergeTimeout", mergeTimeout);
    s.setString("timezones_config", timezones_config);
    s.setBool("smartMultipartForward", smartMultipartForward);
    s.setBool("srcSmeSeparateRouting", srcSmeSeparateRouting);
    s.setInt("schedulerSoftLimit", schedulerSoftLimit);
    s.setInt("schedulerHardLimit", schedulerHardLimit);

    // sms
    s = c.getOrCreateSection("sms");
    s.setInt("max_valid_time", smsMaxValidTime);

    // map
    s = c.getOrCreateSection("map");
    s.setInt("busyMTDelay", mapBusyMTDelay);
    s.setInt("lockedByMODelay", mapLockedByMODelay);
    s.setInt("MOLockTimeout", mapMOLockTimeout);
    s.setBool("allowCallBarred", mapAllowCallBarred);
    s.setBool("ussdV1Enabled", mapUssdV1Enabled);
    s.setBool("ussdV1UseOrigEntityNumber", mapUssdV1UseOrigEntityNumber);
    s.setInt("mapIOTasksCount", mapIOTasksCount);

    //trafficControl
    s = c.getOrCreateSection("trafficControl");
    s.setInt("shapeTimeFrame", trafficShapeTimeFrame);
    s.setInt("statTimeFrame", trafficStatTimeFrame);

    //MessageStore
    s = c.getOrCreateSection("MessageStore");
    s.setInt("archiveInterval", msArchiveInterval);

    //MessageStore.LocalStore
    s = s.getOrCreateSection("LocalStore");
    s.setInt("maxStoreSize", lsMaxStoreSize);
    s.setInt("minRollTime", lsMinRollTime);

    //smpp
    s = c.getOrCreateSection("smpp");
    s.setInt("readTimeout", smppReadTimeout);
    s.setInt("inactivityTime", smppInactivityTime);
    s.setInt("inactivityTimeOut", smppInactivityTimeOut);
    s.setInt("bindTimeout", smppBindTimeout);
    s.setInt("defaultConnectionsLimit", smppDefaultConnectionsLimit);

    //profiler
    s = c.getOrCreateSection("profiler");
    s.setString("systemId", profilerSystemId);
    s.setString("service_type", profilerServiceType);
    s.setInt("protocol_id", profilerProtocolId);
    s.setString("storeFile", profilerStoreFile);

    //profiler.ussdOpsMapping
    s = s.getOrCreateSection("ussdOpsMapping");

    s.setInt("REPORT NONE", profilerReportNone);
    s.setInt("REPORT FULL", profilerReportFull);
    s.setInt("REPORT FINAL", profilerReportFinal);
    s.setInt("LOCALE EN_EN", profilerLocaleEN);
    s.setInt("LOCALE RU_RU", profilerLocaleRU);
    s.setInt("DEFAULT", profilerDefault);
    s.setInt("UCS2", profilerUCS2);
    s.setInt("HIDE", profilerHide);
    s.setInt("UNHIDE", profilerUnhide);
    s.setInt("USSD7BIT ON", profilerUSSD7BitOn);
    s.setInt("USSD7BIT OFF", profilerUSSD7BitOff);
    s.setInt("DIVERT ON", profilerDivertOn);
    s.setInt("DIVERT OFF", profilerDivertOff);
    s.setInt("DIVERT ABSENT ON", profilerDivertAbsentOn);
    s.setInt("DIVERT ABSENT OFF", profilerDivertAbsentOff);
    s.setInt("DIVERT BLOCKED ON", profilerDivertBlockedOn);
    s.setInt("DIVERT BLOCKED OFF", profilerDivertBlockedOff);
    s.setInt("DIVERT BARRED ON", profilerDivertBarredOn);
    s.setInt("DIVERT BARRED OFF", profilerDivertBarredOff);
    s.setInt("DIVERT CAPACITY ON", profilerDivertCapacityOn);
    s.setInt("DIVERT CAPACITY OFF", profilerDivertCapacityOff);
    s.setInt("DIVERT STATUS", profilerDivertStatus);
    s.setInt("DIVERT TO", profilerDivertTo);
    s.setInt("CONCAT ON", profilerConcatOn);
    s.setInt("CONCAT OFF", profilerConcatOff);
    s.setInt("TRANSLIT ON", profilerTranslitOn);
    s.setInt("TRANSLIT OFF", profilerTranslitOff);

    //abonentinfo
    s = c.getOrCreateSection("abonentinfo");
    s.setString("systemId", abInfoSystemId);
    s.setString("service_type", abInfoServiceType);
    s.setInt("protocol_id", abInfoProtocolId);
    s.setString("mobile_access_address", abInfoMobileAccessAddress);
    s.setString("smpp_access_address", abInfoSmppAccessAddress);

    //MscManager
    s = c.getOrCreateSection("MscManager");
    s.setBool("automaticRegistration", mscAutomaticRegistration);
    s.setInt("failureLimit", mscFailureLimit);
    s.setString("storeFile", mscStoreFile);
    s.setInt("singleAttemptTimeout", mscSingleAttemptTimeout);

    //acl
    s = c.getOrCreateSection("acl");
    s.setString("storeDir", aclStoreDir);
    s.setInt("preCreateSize", aclPreCreateSize);

    //inman
    s = c.getOrCreateSection("inman");
    s.setString("host", inmanHost);
    s.setInt("port", inmanPort);


    //inman.chargingPolicy
    s = s.getOrCreateSection("chargingPolicy");
    s.setString("peer2peer", inmanChargingPeer2peer);
    s.setString("other", inmanChargingOther);

    //aliasman
    s = c.getOrCreateSection("aliasman");
    s.setString("storeFile", aliasStoreFile);

    //snmp
    s = c.getOrCreateSection("snmp");
    s.setString("csvFileDir", snmpCsvFileDir);
    s.setInt("csvFileRollInterval", snmpCsvFileRollInterval);
    s.setInt("cacheTimeout", snmpCacheTimeout);

    //directives
    s = c.getOrCreateSection("directives");
    s.clear();
    for (Map.Entry<String, String> e : directives.entrySet()) {
      s.setString(e.getKey(), e.getValue());
    }
  }

  protected void load(XmlConfig c) throws XmlConfigException, AdminException {
    // core
    XmlConfigSection s = c.getSection("core");
    state_machines_count = s.getInt("state_machines_count");
    mainLoopsCount = s.getInt("mainLoopsCount");
    eventQueueLimit = s.getInt("eventQueueLimit");
    service_center_address = s.getString("service_center_address");
    ussd_center_address = s.getString("ussd_center_address");
    ussd_ssn = s.getInt("ussd_ssn");
    add_ussd_ssn = s.getStringArray("add_ussd_ssn", ",");
    systemId = s.getString("systemId");
    service_type = s.getString("service_type");
    protocol_id = s.getInt("protocol_id");
    locales = s.getStringArray("locales", ",");
    default_locale = s.getString("default_locale");
    mergeTimeout = s.getInt("mergeTimeout");
    timezones_config = s.getString("timezones_config");
    smartMultipartForward = s.getBool("smartMultipartForward");
    srcSmeSeparateRouting = s.getBool("srcSmeSeparateRouting");
    schedulerSoftLimit = s.getInt("schedulerSoftLimit");
    schedulerHardLimit = s.getInt("schedulerHardLimit");

    // sms
    s = c.getSection("sms");
    smsMaxValidTime = s.getInt("max_valid_time");

    // map
    s = c.getSection("map");
    mapBusyMTDelay = s.getInt("busyMTDelay");
    mapLockedByMODelay = s.getInt("lockedByMODelay");
    mapMOLockTimeout = s.getInt("MOLockTimeout");
    mapAllowCallBarred = s.getBool("allowCallBarred");
    mapUssdV1Enabled = s.getBool("ussdV1Enabled");
    mapUssdV1UseOrigEntityNumber = s.getBool("ussdV1UseOrigEntityNumber");
    mapIOTasksCount = s.getInt("mapIOTasksCount");

    //trafficControl
    s = c.getSection("trafficControl");
    trafficShapeTimeFrame = s.getInt("shapeTimeFrame");
    trafficStatTimeFrame = s.getInt("statTimeFrame");

    //MessageStore
    s = c.getSection("MessageStore");
    msArchiveInterval = s.getInt("archiveInterval");

    //MessageStore.LocalStore
    s = c.getSection("MessageStore").getSection("LocalStore");
    lsMaxStoreSize = s.getInt("maxStoreSize");
    lsMinRollTime = s.getInt("minRollTime");

    //smpp
    s = c.getSection("smpp");
    smppReadTimeout = s.getInt("readTimeout");
    smppInactivityTime = s.getInt("inactivityTime");
    smppInactivityTimeOut = s.getInt("inactivityTimeOut");
    smppBindTimeout = s.getInt("bindTimeout");
    smppDefaultConnectionsLimit = s.getInt("defaultConnectionsLimit");

    //profiler
    s = c.getSection("profiler");
    profilerSystemId = s.getString("systemId");
    profilerServiceType = s.getString("service_type");
    profilerProtocolId = s.getInt("protocol_id");
    profilerStoreFile = s.getString("storeFile");

    //profiler.ussdOpsMapping
    s = s.getSection("ussdOpsMapping");

    profilerReportNone = s.getInt("REPORT NONE");
    profilerReportFull = s.getInt("REPORT FULL");
    profilerReportFinal = s.getInt("REPORT FINAL");
    profilerLocaleEN = s.getInt("LOCALE EN_EN");
    profilerLocaleRU = s.getInt("LOCALE RU_RU");
    profilerDefault = s.getInt("DEFAULT");
    profilerUCS2 = s.getInt("UCS2");
    profilerHide = s.getInt("HIDE");
    profilerUnhide = s.getInt("UNHIDE");
    profilerUSSD7BitOn = s.getInt("USSD7BIT ON");
    profilerUSSD7BitOff = s.getInt("USSD7BIT OFF");
    profilerDivertOn = s.getInt("DIVERT ON");
    profilerDivertOff = s.getInt("DIVERT OFF");
    profilerDivertAbsentOn = s.getInt("DIVERT ABSENT ON");
    profilerDivertAbsentOff = s.getInt("DIVERT ABSENT OFF");
    profilerDivertBlockedOn = s.getInt("DIVERT BLOCKED ON");
    profilerDivertBlockedOff = s.getInt("DIVERT BLOCKED OFF");
    profilerDivertBarredOn = s.getInt("DIVERT BARRED ON");
    profilerDivertBarredOff = s.getInt("DIVERT BARRED OFF");
    profilerDivertCapacityOn = s.getInt("DIVERT CAPACITY ON");
    profilerDivertCapacityOff = s.getInt("DIVERT CAPACITY OFF");
    profilerDivertStatus = s.getInt("DIVERT STATUS");
    profilerDivertTo = s.getInt("DIVERT TO");
    profilerConcatOn = s.getInt("CONCAT ON");
    profilerConcatOff = s.getInt("CONCAT OFF");
    profilerTranslitOn = s.getInt("TRANSLIT ON");
    profilerTranslitOff = s.getInt("TRANSLIT OFF");

    //abonentinfo
    s = c.getSection("abonentinfo");
    abInfoSystemId = s.getString("systemId");
    abInfoServiceType = s.getString("service_type");
    abInfoProtocolId = s.getInt("protocol_id");
    abInfoMobileAccessAddress = s.getString("mobile_access_address");
    abInfoSmppAccessAddress = s.getString("smpp_access_address");

    //MscManager
    s = c.getSection("MscManager");
    mscAutomaticRegistration = s.getBool("automaticRegistration");
    mscFailureLimit = s.getInt("failureLimit");
    mscStoreFile = s.getString("storeFile");
    mscSingleAttemptTimeout = s.getInt("singleAttemptTimeout");

    //acl
    s = c.getSection("acl");
    aclStoreDir = s.getString("storeDir");
    aclPreCreateSize = s.getInt("preCreateSize");

    //inman
    s = c.getSection("inman");
    inmanHost = s.getString("host");
    inmanPort = s.getInt("port");


    //inman.chargingPolicy
    s = s.getSection("chargingPolicy");
    inmanChargingPeer2peer = s.getString("peer2peer");
    inmanChargingOther = s.getString("other");

    //aliasman
    s = c.getSection("aliasman");
    aliasStoreFile = s.getString("storeFile");

    //snmp
    s = c.getSection("snmp");
    snmpCsvFileDir = s.getString("csvFileDir");
    snmpCsvFileRollInterval = s.getInt("csvFileRollInterval");
    snmpCacheTimeout = s.getInt("cacheTimeout");

    //directives
    if (c.containsSection("directives")) {
      Map<String, String> directives = new HashMap<String, String>();
      s = c.getSection("directives");
      for (XmlConfigParam p : s.params()) {
        directives.put(p.getName(), p.getString());
      }
      setDirectives(directives);
    }
  }

  public int getState_machines_count() {
    return state_machines_count;
  }

  public void setState_machines_count(int state_machines_count) throws AdminException {
    vh.checkPositive("state_machines_count", state_machines_count);
    this.state_machines_count = state_machines_count;
  }

  public int getMainLoopsCount() {
    return mainLoopsCount;
  }

  public void setMainLoopsCount(int mainLoopsCount) throws AdminException {
    vh.checkPositive("mainLoopsCount", mainLoopsCount);
    this.mainLoopsCount = mainLoopsCount;
  }

  public int getEventQueueLimit() {
    return eventQueueLimit;
  }

  public void setEventQueueLimit(int eventQueueLimit) throws AdminException {
    vh.checkPositive("eventQueueLimit", mainLoopsCount);
    this.eventQueueLimit = eventQueueLimit;
  }

  public String getService_center_address() {
    return service_center_address;
  }

  public void setService_center_address(String service_center_address) throws AdminException {
    vh.checkNotEmpty("service_center_address", service_center_address);
    this.service_center_address = service_center_address;
  }

  public String getUssd_center_address() {
    return ussd_center_address;
  }

  public void setUssd_center_address(String ussd_center_address) throws AdminException {
    vh.checkNotEmpty("ussd_center_address", ussd_center_address);
    this.ussd_center_address = ussd_center_address;
  }

  public int getUssd_ssn() {
    return ussd_ssn;
  }

  public void setUssd_ssn(int ussd_ssn) throws AdminException {
    vh.checkPositive("ussd_ssn", ussd_ssn);
    this.ussd_ssn = ussd_ssn;
  }

  public String[] getAdd_ussd_ssn() {
    return add_ussd_ssn;
  }

  public void setAdd_ussd_ssn(String[] add_ussd_ssn) throws AdminException {
    vh.checkNotEmpty("add_ussd_ssn", add_ussd_ssn);
    this.add_ussd_ssn = add_ussd_ssn;
  }

  public String getSystemId() {
    return systemId;
  }

  public void setSystemId(String systemId) throws AdminException {
    vh.checkNotEmpty("systemId", systemId);
    this.systemId = systemId;
  }

  public String getService_type() {
    return service_type;
  }

  public void setService_type(String service_type) throws AdminException {
    vh.checkNotEmpty("service_type", service_type);
    this.service_type = service_type;
  }

  public int getProtocol_id() {
    return protocol_id;
  }

  public void setProtocol_id(int protocol_id) throws AdminException {
    vh.checkPositive("protocol_id", protocol_id);
    this.protocol_id = protocol_id;
  }

  public String[] getLocales() {
    return locales;
  }

  public void setLocales(String[] locales) throws AdminException {
    vh.checkNotEmpty("locales", locales);
    this.locales = locales;
  }

  public String getDefault_locale() {
    return default_locale;
  }

  public void setDefault_locale(String default_locale) throws AdminException {
    vh.checkNotEmpty("default_locale", default_locale);
    this.default_locale = default_locale;
  }

  public int getMergeTimeout() {
    return mergeTimeout;
  }

  public void setMergeTimeout(int mergeTimeout) throws AdminException {
    vh.checkPositive("mergeTimeout", mergeTimeout);
    this.mergeTimeout = mergeTimeout;
  }

  public String getTimezones_config() {
    return timezones_config;
  }

  public void setTimezones_config(String timezones_config) throws AdminException {
    vh.checkNotEmpty("timezones_config", timezones_config);
    this.timezones_config = timezones_config;
  }

  public boolean isSmartMultipartForward() {
    return smartMultipartForward;
  }

  public void setSmartMultipartForward(boolean smartMultipartForward) throws AdminException {
    this.smartMultipartForward = smartMultipartForward;
  }

  public boolean isSrcSmeSeparateRouting() {
    return srcSmeSeparateRouting;
  }

  public void setSrcSmeSeparateRouting(boolean srcSmeSeparateRouting) throws AdminException {
    this.srcSmeSeparateRouting = srcSmeSeparateRouting;
  }

  public int getSchedulerSoftLimit() {
    return schedulerSoftLimit;
  }

  public void setSchedulerSoftLimit(int schedulerSoftLimit) throws AdminException {
    vh.checkPositive("schedulerSoftLimit", schedulerSoftLimit);
    this.schedulerSoftLimit = schedulerSoftLimit;
  }

  public int getSchedulerHardLimit() {
    return schedulerHardLimit;
  }

  public void setSchedulerHardLimit(int schedulerHardLimit) throws AdminException {
    vh.checkPositive("schedulerHardLimit", schedulerHardLimit);
    this.schedulerHardLimit = schedulerHardLimit;
  }

  public int getSmsMaxValidTime() {
    return smsMaxValidTime;
  }

  public void setSmsMaxValidTime(int smsMaxValidTime) throws AdminException {
    vh.checkPositive("smsMaxValidTime", smsMaxValidTime);
    this.smsMaxValidTime = smsMaxValidTime;
  }

  public int getMapBusyMTDelay() {
    return mapBusyMTDelay;
  }

  public void setMapBusyMTDelay(int mapBusyMTDelay) throws AdminException {
    vh.checkPositive("mapBusyMTDelay", mapBusyMTDelay);
    this.mapBusyMTDelay = mapBusyMTDelay;
  }

  public int getMapLockedByMODelay() {
    return mapLockedByMODelay;
  }

  public void setMapLockedByMODelay(int mapLockedByMODelay) throws AdminException {
    vh.checkPositive("mapLockedByMODelay", mapLockedByMODelay);
    this.mapLockedByMODelay = mapLockedByMODelay;
  }

  public int getMapMOLockTimeout() {
    return mapMOLockTimeout;
  }

  public void setMapMOLockTimeout(int mapMOLockTimeout) throws AdminException {
    vh.checkPositive("mapMOLockTimeout", mapMOLockTimeout);
    this.mapMOLockTimeout = mapMOLockTimeout;
  }

  public boolean isMapAllowCallBarred() {
    return mapAllowCallBarred;
  }

  public void setMapAllowCallBarred(boolean mapAllowCallBarred) throws AdminException {
    this.mapAllowCallBarred = mapAllowCallBarred;
  }

  public boolean isMapUssdV1Enabled() {
    return mapUssdV1Enabled;
  }

  public void setMapUssdV1Enabled(boolean mapUssdV1Enabled) throws AdminException {
    this.mapUssdV1Enabled = mapUssdV1Enabled;
  }

  public boolean isMapUssdV1UseOrigEntityNumber() {
    return mapUssdV1UseOrigEntityNumber;
  }

  public void setMapUssdV1UseOrigEntityNumber(boolean mapUssdV1UseOrigEntityNumber) throws AdminException {
    this.mapUssdV1UseOrigEntityNumber = mapUssdV1UseOrigEntityNumber;
  }

  public int getMapIOTasksCount() {
    return mapIOTasksCount;
  }

  public void setMapIOTasksCount(int mapIOTasksCount) throws AdminException {
    vh.checkPositive("mapIOTasksCount", mapIOTasksCount);
    this.mapIOTasksCount = mapIOTasksCount;
  }

  public int getTrafficShapeTimeFrame() {
    return trafficShapeTimeFrame;
  }

  public void setTrafficShapeTimeFrame(int trafficShapeTimeFrame) throws AdminException {
    vh.checkPositive("trafficShapeTimeFrame", trafficShapeTimeFrame);
    this.trafficShapeTimeFrame = trafficShapeTimeFrame;
  }

  public int getTrafficStatTimeFrame() {
    return trafficStatTimeFrame;
  }

  public void setTrafficStatTimeFrame(int trafficStatTimeFrame) throws AdminException {
    vh.checkPositive("trafficStatTimeFrame", trafficStatTimeFrame);
    this.trafficStatTimeFrame = trafficStatTimeFrame;
  }

  public int getMsArchiveInterval() {
    return msArchiveInterval;
  }

  public void setMsArchiveInterval(int msArchiveInterval) throws AdminException {
    vh.checkPositive("msArchiveInterval", msArchiveInterval);
    this.msArchiveInterval = msArchiveInterval;
  }

  public int getLsMaxStoreSize() {
    return lsMaxStoreSize;
  }

  public void setLsMaxStoreSize(int lsMaxStoreSize) throws AdminException {
    vh.checkPositive("lsMaxStoreSize", lsMaxStoreSize);
    this.lsMaxStoreSize = lsMaxStoreSize;
  }

  public int getLsMinRollTime() {
    return lsMinRollTime;
  }

  public void setLsMinRollTime(int lsMinRollTime) throws AdminException {
    vh.checkPositive("lsMinRollTime", lsMinRollTime);
    this.lsMinRollTime = lsMinRollTime;
  }

  public int getSmppReadTimeout() {
    return smppReadTimeout;
  }

  public void setSmppReadTimeout(int smppReadTimeout) throws AdminException {
    vh.checkPositive("smppReadTimeout", smppReadTimeout);
    this.smppReadTimeout = smppReadTimeout;
  }

  public int getSmppInactivityTime() {
    return smppInactivityTime;
  }

  public void setSmppInactivityTime(int smppInactivityTime) throws AdminException {
    vh.checkPositive("smppInactivityTime", smppInactivityTime);
    this.smppInactivityTime = smppInactivityTime;
  }

  public int getSmppInactivityTimeOut() {
    return smppInactivityTimeOut;
  }

  public void setSmppInactivityTimeOut(int smppInactivityTimeOut) throws AdminException {
    vh.checkPositive("smppInactivityTimeOut", smppInactivityTimeOut);
    this.smppInactivityTimeOut = smppInactivityTimeOut;
  }

  public int getSmppBindTimeout() {
    return smppBindTimeout;
  }

  public void setSmppBindTimeout(int smppBindTimeout) throws AdminException {
    vh.checkPositive("smppBindTimeout", smppBindTimeout);
    this.smppBindTimeout = smppBindTimeout;
  }

  public int getSmppDefaultConnectionsLimit() {
    return smppDefaultConnectionsLimit;
  }

  public void setSmppDefaultConnectionsLimit(int smppDefaultConnectionsLimit) throws AdminException {
    vh.checkPositive("smppDefaultConnectionsLimit", smppDefaultConnectionsLimit);
    this.smppDefaultConnectionsLimit = smppDefaultConnectionsLimit;
  }

  public String getProfilerSystemId() {
    return profilerSystemId;
  }

  public void setProfilerSystemId(String profilerSystemId) throws AdminException {
    vh.checkNotEmpty("profilerSystemId", profilerSystemId);
    this.profilerSystemId = profilerSystemId;
  }

  public String getProfilerServiceType() {
    return profilerServiceType;
  }

  public void setProfilerServiceType(String profilerServiceType) throws AdminException {
    vh.checkNotEmpty("profilerServiceType", profilerServiceType);
    this.profilerServiceType = profilerServiceType;
  }

  public int getProfilerProtocolId() {
    return profilerProtocolId;
  }

  public void setProfilerProtocolId(int profilerProtocolId) throws AdminException {
    vh.checkPositive("profilerProtocolId", profilerProtocolId);
    this.profilerProtocolId = profilerProtocolId;
  }

  public String getProfilerStoreFile() {
    return profilerStoreFile;
  }

  public void setProfilerStoreFile(String profilerStoreFile)throws AdminException  {
    vh.checkNotEmpty("profilerStoreFile", profilerStoreFile);
    this.profilerStoreFile = profilerStoreFile;
  }

  public int getProfilerReportNone() {
    return profilerReportNone;
  }

  public void setProfilerReportNone(int profilerReportNone) throws AdminException {
    vh.checkPositive("profilerReportNone", profilerReportNone);
    this.profilerReportNone = profilerReportNone;
  }

  public int getProfilerReportFull() {
    return profilerReportFull;
  }

  public void setProfilerReportFull(int profilerReportFull) throws AdminException {
    vh.checkPositive("profilerReportFull", profilerReportFull);
    this.profilerReportFull = profilerReportFull;
  }

  public int getProfilerReportFinal() {
    return profilerReportFinal;
  }

  public void setProfilerReportFinal(int profilerReportFinal) throws AdminException {
    vh.checkPositive("profilerReportFinal", profilerReportFinal);
    this.profilerReportFinal = profilerReportFinal;
  }

  public int getProfilerLocaleRU() {
    return profilerLocaleRU;
  }

  public void setProfilerLocaleRU(int profilerLocaleRU) throws AdminException {
    vh.checkPositive("profilerLocaleRU", profilerLocaleRU);
    this.profilerLocaleRU = profilerLocaleRU;
  }

  public int getProfilerLocaleEN() {
    return profilerLocaleEN;
  }

  public void setProfilerLocaleEN(int profilerLocaleEN) throws AdminException {
    vh.checkPositive("profilerLocaleEN", profilerLocaleEN);
    this.profilerLocaleEN = profilerLocaleEN;
  }

  public int getProfilerDefault() {
    return profilerDefault;
  }

  public void setProfilerDefault(int profilerDefault) throws AdminException {
    vh.checkPositive("profilerDefault", profilerDefault);
    this.profilerDefault = profilerDefault;
  }

  public int getProfilerUCS2() {
    return profilerUCS2;
  }

  public void setProfilerUCS2(int profilerUCS2)throws AdminException  {
    vh.checkPositive("profilerUCS2", profilerUCS2);
    this.profilerUCS2 = profilerUCS2;
  }

  public int getProfilerHide() {
    return profilerHide;
  }

  public void setProfilerHide(int profilerHide) throws AdminException {
    vh.checkPositive("profilerHide", profilerHide);
    this.profilerHide = profilerHide;
  }

  public int getProfilerUnhide() {
    return profilerUnhide;
  }

  public void setProfilerUnhide(int profilerUnhide) throws AdminException {
    vh.checkPositive("profilerUnhide", profilerUnhide);
    this.profilerUnhide = profilerUnhide;
  }

  public int getProfilerUSSD7BitOn() {
    return profilerUSSD7BitOn;
  }

  public void setProfilerUSSD7BitOn(int profilerUSSD7BitOn) throws AdminException {
    vh.checkPositive("profilerUSSD7BitOn", profilerUSSD7BitOn);
    this.profilerUSSD7BitOn = profilerUSSD7BitOn;
  }

  public int getProfilerUSSD7BitOff() {
    return profilerUSSD7BitOff;
  }

  public void setProfilerUSSD7BitOff(int profilerUSSD7BitOff) throws AdminException {
    vh.checkPositive("profilerUSSD7BitOff", profilerUSSD7BitOff);
    this.profilerUSSD7BitOff = profilerUSSD7BitOff;
  }

  public int getProfilerDivertOn() {
    return profilerDivertOn;
  }

  public void setProfilerDivertOn(int profilerDivertOn) throws AdminException {
    vh.checkPositive("profilerDivertOn", profilerDivertOn);
    this.profilerDivertOn = profilerDivertOn;
  }

  public int getProfilerDivertOff() {
    return profilerDivertOff;
  }

  public void setProfilerDivertOff(int profilerDivertOff) throws AdminException {
    vh.checkPositive("profilerDivertOff", profilerDivertOff);
    this.profilerDivertOff = profilerDivertOff;
  }

  public int getProfilerDivertAbsentOn() {
    return profilerDivertAbsentOn;
  }

  public void setProfilerDivertAbsentOn(int profilerDivertAbsentOn) throws AdminException {
    vh.checkPositive("profilerDivertAbsentOn", profilerDivertAbsentOn);
    this.profilerDivertAbsentOn = profilerDivertAbsentOn;
  }

  public int getProfilerDivertAbsentOff() {
    return profilerDivertAbsentOff;
  }

  public void setProfilerDivertAbsentOff(int profilerDivertAbsentOff) throws AdminException {
    vh.checkPositive("profilerDivertAbsentOff", profilerDivertAbsentOff);
    this.profilerDivertAbsentOff = profilerDivertAbsentOff;
  }

  public int getProfilerDivertBlockedOn() {
    return profilerDivertBlockedOn;
  }

  public void setProfilerDivertBlockedOn(int profilerDivertBlockedOn) throws AdminException {
    vh.checkPositive("profilerDivertBlockedOn", profilerDivertBlockedOn);
    this.profilerDivertBlockedOn = profilerDivertBlockedOn;
  }

  public int getProfilerDivertBlockedOff() {
    return profilerDivertBlockedOff;
  }

  public void setProfilerDivertBlockedOff(int profilerDivertBlockedOff) throws AdminException {
    vh.checkPositive("profilerDivertBlockedOff", profilerDivertBlockedOff);
    this.profilerDivertBlockedOff = profilerDivertBlockedOff;
  }

  public int getProfilerDivertBarredOn() {
    return profilerDivertBarredOn;
  }

  public void setProfilerDivertBarredOn(int profilerDivertBarredOn) throws AdminException {
    vh.checkPositive("profilerDivertBarredOn", profilerDivertBarredOn);
    this.profilerDivertBarredOn = profilerDivertBarredOn;
  }

  public int getProfilerDivertBarredOff() {
    return profilerDivertBarredOff;
  }

  public void setProfilerDivertBarredOff(int profilerDivertBarredOff) throws AdminException {
    vh.checkPositive("profilerDivertBarredOff", profilerDivertBarredOff);
    this.profilerDivertBarredOff = profilerDivertBarredOff;
  }

  public int getProfilerDivertCapacityOn() {
    return profilerDivertCapacityOn;
  }

  public void setProfilerDivertCapacityOn(int profilerDivertCapacityOn) throws AdminException {
    vh.checkPositive("profilerDivertCapacityOn", profilerDivertCapacityOn);
    this.profilerDivertCapacityOn = profilerDivertCapacityOn;
  }

  public int getProfilerDivertCapacityOff() {
    return profilerDivertCapacityOff;
  }

  public void setProfilerDivertCapacityOff(int profilerDivertCapacityOff) throws AdminException {
    vh.checkPositive("profilerDivertCapacityOff", profilerDivertCapacityOff);
    this.profilerDivertCapacityOff = profilerDivertCapacityOff;
  }

  public int getProfilerDivertStatus() {
    return profilerDivertStatus;
  }

  public void setProfilerDivertStatus(int profilerDivertStatus) throws AdminException {
    vh.checkPositive("profilerDivertStatus", profilerDivertStatus);
    this.profilerDivertStatus = profilerDivertStatus;
  }

  public int getProfilerDivertTo() {
    return profilerDivertTo;
  }

  public void setProfilerDivertTo(int profilerDivertTo) throws AdminException {
    vh.checkPositive("profilerDivertTo",profilerDivertTo);
    this.profilerDivertTo = profilerDivertTo;
  }

  public int getProfilerConcatOn() {
    return profilerConcatOn;
  }

  public void setProfilerConcatOn(int profilerConcatOn) throws AdminException {
    vh.checkPositive("profilerConcatOn", profilerConcatOn);
    this.profilerConcatOn = profilerConcatOn;
  }

  public int getProfilerConcatOff() {
    return profilerConcatOff;
  }

  public void setProfilerConcatOff(int profilerConcatOff) throws AdminException {
    vh.checkPositive("profilerConcatOff", profilerConcatOff);
    this.profilerConcatOff = profilerConcatOff;
  }

  public int getProfilerTranslitOn() {
    return profilerTranslitOn;
  }

  public void setProfilerTranslitOn(int profilerTranslitOn) throws AdminException {
    vh.checkPositive("profilerTranslitOn", profilerTranslitOn);
    this.profilerTranslitOn = profilerTranslitOn;
  }

  public int getProfilerTranslitOff() {
    return profilerTranslitOff;
  }

  public void setProfilerTranslitOff(int profilerTranslitOff) throws AdminException {
    vh.checkPositive("profilerTranslitOff", profilerTranslitOff);
    this.profilerTranslitOff = profilerTranslitOff;
  }

  public String getAbInfoSystemId() {
    return abInfoSystemId;
  }

  public void setAbInfoSystemId(String abInfoSystemId) throws AdminException {
    vh.checkNotEmpty("abInfoSystemId", abInfoSystemId);
    this.abInfoSystemId = abInfoSystemId;
  }

  public String getAbInfoServiceType() {
    return abInfoServiceType;
  }

  public void setAbInfoServiceType(String abInfoServiceType) throws AdminException {
    vh.checkNotEmpty("abInfoServiceType", abInfoServiceType);
    this.abInfoServiceType = abInfoServiceType;
  }

  public int getAbInfoProtocolId() {
    return abInfoProtocolId;
  }

  public void setAbInfoProtocolId(int abInfoProtocolId) throws AdminException {
    vh.checkPositive("abInfoProtocolId", abInfoProtocolId);
    this.abInfoProtocolId = abInfoProtocolId;
  }

  public String getAbInfoMobileAccessAddress() {
    return abInfoMobileAccessAddress;
  }

  public void setAbInfoMobileAccessAddress(String abInfoMobileAccessAddress) throws AdminException {
    vh.checkNotEmpty("abInfoMobileAccessAddress", abInfoMobileAccessAddress);
    this.abInfoMobileAccessAddress = abInfoMobileAccessAddress;
  }

  public String getAbInfoSmppAccessAddress() {
    return abInfoSmppAccessAddress;
  }

  public void setAbInfoSmppAccessAddress(String abInfoSmppAccessAddress) throws AdminException {
    vh.checkNotEmpty("abInfoSmppAccessAddress", abInfoSmppAccessAddress);
    this.abInfoSmppAccessAddress = abInfoSmppAccessAddress;
  }

  public boolean isMscAutomaticRegistration() {
    return mscAutomaticRegistration;
  }

  public void setMscAutomaticRegistration(boolean mscAutomaticRegistration) throws AdminException {
    this.mscAutomaticRegistration = mscAutomaticRegistration;
  }

  public int getMscFailureLimit() {
    return mscFailureLimit;
  }

  public void setMscFailureLimit(int mscFailureLimit) throws AdminException {
    vh.checkPositive("mscFailureLimit", mscFailureLimit);
    this.mscFailureLimit = mscFailureLimit;
  }

  public String getMscStoreFile() {
    return mscStoreFile;
  }

  public void setMscStoreFile(String mscStoreFile) throws AdminException {
    vh.checkNotEmpty("mscStoreFile", mscStoreFile);
    this.mscStoreFile = mscStoreFile;
  }

  public int getMscSingleAttemptTimeout() {
    return mscSingleAttemptTimeout;
  }

  public void setMscSingleAttemptTimeout(int mscSingleAttemptTimeout) throws AdminException {
    vh.checkPositive("mscSingleAttemptTimeout", mscSingleAttemptTimeout);
    this.mscSingleAttemptTimeout = mscSingleAttemptTimeout;
  }

  public String getAclStoreDir() {
    return aclStoreDir;
  }

  public void setAclStoreDir(String aclStoreDir) throws AdminException {
    vh.checkNotEmpty("aclStoreDir", aclStoreDir);
    this.aclStoreDir = aclStoreDir;
  }

  public int getAclPreCreateSize() {
    return aclPreCreateSize;
  }

  public void setAclPreCreateSize(int aclPreCreateSize) throws AdminException {
    vh.checkPositive("aclPreCreateSize", aclPreCreateSize);
    this.aclPreCreateSize = aclPreCreateSize;
  }

  public String getInmanHost() {
    return inmanHost;
  }

  public void setInmanHost(String inmanHost) throws AdminException {
    vh.checkNotEmpty("inmanHost", inmanHost);
    this.inmanHost = inmanHost;
  }

  public int getInmanPort() {
    return inmanPort;
  }

  public void setInmanPort(int inmanPort) throws AdminException {
    vh.checkPositive("inmanPort", inmanPort);
    this.inmanPort = inmanPort;
  }

  public String getInmanChargingPeer2peer() {
    return inmanChargingPeer2peer;
  }

  public void setInmanChargingPeer2peer(String inmanChargingPeer2peer) throws AdminException {
    vh.checkNotEmpty("inmanChargingPeer2peer", inmanChargingPeer2peer);
    this.inmanChargingPeer2peer = inmanChargingPeer2peer;
  }

  public String getInmanChargingOther() {
    return inmanChargingOther;
  }

  public void setInmanChargingOther(String inmanChargingOther) throws AdminException {
    vh.checkNotEmpty("inmanChargingOther", inmanChargingOther);
    this.inmanChargingOther = inmanChargingOther;
  }

  public String getAliasStoreFile() {
    return aliasStoreFile;
  }

  public void setAliasStoreFile(String aliasStoreFile) throws AdminException {
    vh.checkNotEmpty("aliasStoreFile", aliasStoreFile);
    this.aliasStoreFile = aliasStoreFile;
  }

  public String getSnmpCsvFileDir() {
    return snmpCsvFileDir;
  }

  public void setSnmpCsvFileDir(String snmpCsvFileDir) throws AdminException {
    vh.checkNotEmpty("snmpCsvFileDir", snmpCsvFileDir);
    this.snmpCsvFileDir = snmpCsvFileDir;
  }

  public int getSnmpCsvFileRollInterval() {
    return snmpCsvFileRollInterval;
  }

  public void setSnmpCsvFileRollInterval(int snmpCsvFileRollInterval) throws AdminException {
    vh.checkPositive("snmpCsvFileRollInterval", snmpCsvFileRollInterval);
    this.snmpCsvFileRollInterval = snmpCsvFileRollInterval;
  }

  public int getSnmpCacheTimeout() {
    return snmpCacheTimeout;
  }

  public void setSnmpCacheTimeout(int snmpCacheTimeout) throws AdminException {
    vh.checkPositive("snmpCacheTimeout", snmpCacheTimeout);
    this.snmpCacheTimeout = snmpCacheTimeout;
  }

  public Map<String, String> getDirectives() {
    return new HashMap<String, String>(directives);
  }

  public void setDirectives(Map<String, String> ds) throws AdminException {
    for (Map.Entry<String, String> directive : ds.entrySet()) {
      if (directive.getValue() == null || directive.getValue().trim().length()==0)
        throw new SmscException("invalid_directive", directive.getKey());
    }
    directives = new HashMap<String, String>(ds);
  }

  public Object clone() throws CloneNotSupportedException {
    CommonSettings cs = (CommonSettings) super.clone();
    // core
    cs.state_machines_count = state_machines_count;
    cs.mainLoopsCount = mainLoopsCount;
    cs.eventQueueLimit = eventQueueLimit;
    cs.service_center_address = service_center_address;
    cs.ussd_center_address = ussd_center_address;
    cs.ussd_ssn = ussd_ssn;
    cs.add_ussd_ssn = new String[add_ussd_ssn.length];
    System.arraycopy(add_ussd_ssn, 0, cs.add_ussd_ssn, 0, add_ussd_ssn.length);
    cs.systemId = systemId;
    cs.service_type = service_type;
    cs.protocol_id = protocol_id;
    cs.locales = new String[locales.length];
    System.arraycopy(locales, 0, cs.locales, 0, locales.length);
    cs.default_locale = default_locale;
    cs.mergeTimeout = mergeTimeout;
    cs.timezones_config = timezones_config;
    cs.smartMultipartForward = smartMultipartForward;
    cs.srcSmeSeparateRouting = srcSmeSeparateRouting;
    cs.schedulerSoftLimit = schedulerSoftLimit;
    cs.schedulerHardLimit = schedulerHardLimit;

    // sms

    cs.smsMaxValidTime = smsMaxValidTime;

    // map

    cs.mapBusyMTDelay = mapBusyMTDelay;
    cs.mapLockedByMODelay = mapLockedByMODelay;
    cs.mapMOLockTimeout = mapMOLockTimeout;
    cs.mapAllowCallBarred = mapAllowCallBarred;
    cs.mapUssdV1Enabled = mapUssdV1Enabled;
    cs.mapUssdV1UseOrigEntityNumber = mapUssdV1UseOrigEntityNumber;
    cs.mapIOTasksCount = mapIOTasksCount;

    // trafficControl

    cs.trafficShapeTimeFrame = trafficShapeTimeFrame;
    cs.trafficStatTimeFrame = trafficStatTimeFrame;

    // MessageStore

    cs.msArchiveInterval = msArchiveInterval;

    // MessageStore.LocalStore

    cs.lsMaxStoreSize = lsMaxStoreSize;
    cs.lsMinRollTime = lsMinRollTime;

    // smpp

    cs.smppReadTimeout = smppReadTimeout;
    cs.smppInactivityTime = smppInactivityTime;
    cs.smppInactivityTimeOut = smppInactivityTimeOut;
    cs.smppBindTimeout = smppBindTimeout;
    cs.smppDefaultConnectionsLimit = smppDefaultConnectionsLimit;

    // profiler

    cs.profilerSystemId = profilerSystemId;
    cs.profilerServiceType = profilerServiceType;
    cs.profilerProtocolId = profilerProtocolId;
    cs.profilerStoreFile = profilerStoreFile;

    //profiler.ussdOpsMapping

    cs.profilerReportNone = profilerReportNone;
    cs.profilerReportFull = profilerReportFull;
    cs.profilerReportFinal = profilerReportFinal;
    cs.profilerLocaleRU = profilerLocaleRU;
    cs.profilerLocaleEN = profilerLocaleEN;
    cs.profilerDefault = profilerDefault;
    cs.profilerUCS2 = profilerUCS2;
    cs.profilerHide = profilerHide;
    cs.profilerUnhide = profilerUnhide;
    cs.profilerUSSD7BitOn = profilerUSSD7BitOn;
    cs.profilerUSSD7BitOff = profilerUSSD7BitOff;
    cs.profilerDivertOn = profilerDivertOn;
    cs.profilerDivertOff = profilerDivertOff;
    cs.profilerDivertAbsentOn = profilerDivertAbsentOn;
    cs.profilerDivertAbsentOff = profilerDivertAbsentOff;
    cs.profilerDivertBlockedOn = profilerDivertBlockedOn;
    cs.profilerDivertBlockedOff = profilerDivertBlockedOff;
    cs.profilerDivertBarredOn = profilerDivertBarredOn;
    cs.profilerDivertBarredOff = profilerDivertBarredOff;
    cs.profilerDivertCapacityOn = profilerDivertCapacityOn;
    cs.profilerDivertCapacityOff = profilerDivertCapacityOff;
    cs.profilerDivertStatus = profilerDivertStatus;
    cs.profilerDivertTo = profilerDivertTo;
    cs.profilerDivertTo = profilerDivertTo;
    cs.profilerConcatOff = profilerConcatOff;
    cs.profilerTranslitOn = profilerTranslitOn;
    cs.profilerTranslitOff = profilerTranslitOff;

    // abonentinfo

    cs.abInfoSystemId = abInfoSystemId;
    cs.abInfoServiceType = abInfoServiceType;
    cs.abInfoProtocolId = abInfoProtocolId;
    cs.abInfoMobileAccessAddress = abInfoMobileAccessAddress;
    cs.abInfoSmppAccessAddress = abInfoSmppAccessAddress;

    // MscManager

    cs.mscAutomaticRegistration = mscAutomaticRegistration;
    cs.mscFailureLimit = mscFailureLimit;
    cs.mscStoreFile = mscStoreFile;
    cs.mscSingleAttemptTimeout = mscSingleAttemptTimeout;

    // acl

    cs.aclStoreDir = aclStoreDir;
    cs.aclPreCreateSize = aclPreCreateSize;

    // inman

    cs.inmanHost = inmanHost;
    cs.inmanPort = inmanPort;

    // inman.chargingPolicy

    cs.inmanChargingPeer2peer = inmanChargingPeer2peer;
    cs.inmanChargingOther = inmanChargingOther;

    // aliasman

    cs.aliasStoreFile = aliasStoreFile;

    // snmp

    cs.snmpCsvFileDir = snmpCsvFileDir;
    cs.snmpCsvFileRollInterval = snmpCsvFileRollInterval;
    cs.snmpCacheTimeout = snmpCacheTimeout;

    // directives

    for (Map.Entry<String, String> e : directives.entrySet()) {
      cs.directives.put(e.getKey(), e.getValue());
    }

    return cs;
  }
}
