package ru.novosoft.smsc.web.beans;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/**
 * author: alkhal
 */
public class CommonConfig implements Serializable {

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

  public int getState_machines_count() {
    return state_machines_count;
  }

  public void setState_machines_count(int state_machines_count) {
    this.state_machines_count = state_machines_count;
  }

  public int getMainLoopsCount() {
    return mainLoopsCount;
  }

  public void setMainLoopsCount(int mainLoopsCount) {
    this.mainLoopsCount = mainLoopsCount;
  }

  public int getEventQueueLimit() {
    return eventQueueLimit;
  }

  public void setEventQueueLimit(int eventQueueLimit) {
    this.eventQueueLimit = eventQueueLimit;
  }

  public String getService_center_address() {
    return service_center_address;
  }

  public void setService_center_address(String service_center_address) {
    this.service_center_address = service_center_address;
  }

  public String getUssd_center_address() {
    return ussd_center_address;
  }

  public void setUssd_center_address(String ussd_center_address) {
    this.ussd_center_address = ussd_center_address;
  }

  public int getUssd_ssn() {
    return ussd_ssn;
  }

  public void setUssd_ssn(int ussd_ssn) {
    this.ussd_ssn = ussd_ssn;
  }

  public String[] getAdd_ussd_ssn() {
    return add_ussd_ssn;
  }

  public void setAdd_ussd_ssn(String[] add_ussd_ssn) {
    this.add_ussd_ssn = add_ussd_ssn;
  }

  public String getSystemId() {
    return systemId;
  }

  public void setSystemId(String systemId) {
    this.systemId = systemId;
  }

  public String getService_type() {
    return service_type;
  }

  public void setService_type(String service_type) {
    this.service_type = service_type;
  }

  public int getProtocol_id() {
    return protocol_id;
  }

  public void setProtocol_id(int protocol_id) {
    this.protocol_id = protocol_id;
  }

  public String[] getLocales() {
    return locales;
  }

  public void setLocales(String[] locales) {
    this.locales = locales;
  }

  public String getDefault_locale() {
    return default_locale;
  }

  public void setDefault_locale(String default_locale) {
    this.default_locale = default_locale;
  }

  public int getMergeTimeout() {
    return mergeTimeout;
  }

  public void setMergeTimeout(int mergeTimeout) {
    this.mergeTimeout = mergeTimeout;
  }

  public String getTimezones_config() {
    return timezones_config;
  }

  public void setTimezones_config(String timezones_config) {
    this.timezones_config = timezones_config;
  }

  public boolean isSmartMultipartForward() {
    return smartMultipartForward;
  }

  public void setSmartMultipartForward(boolean smartMultipartForward) {
    this.smartMultipartForward = smartMultipartForward;
  }

  public boolean isSrcSmeSeparateRouting() {
    return srcSmeSeparateRouting;
  }

  public void setSrcSmeSeparateRouting(boolean srcSmeSeparateRouting) {
    this.srcSmeSeparateRouting = srcSmeSeparateRouting;
  }

  public int getSchedulerSoftLimit() {
    return schedulerSoftLimit;
  }

  public void setSchedulerSoftLimit(int schedulerSoftLimit) {
    this.schedulerSoftLimit = schedulerSoftLimit;
  }

  public int getSchedulerHardLimit() {
    return schedulerHardLimit;
  }

  public void setSchedulerHardLimit(int schedulerHardLimit) {
    this.schedulerHardLimit = schedulerHardLimit;
  }

  public int getSmsMaxValidTime() {
    return smsMaxValidTime;
  }

  public void setSmsMaxValidTime(int smsMaxValidTime) {
    this.smsMaxValidTime = smsMaxValidTime;
  }

  public int getMapBusyMTDelay() {
    return mapBusyMTDelay;
  }

  public void setMapBusyMTDelay(int mapBusyMTDelay) {
    this.mapBusyMTDelay = mapBusyMTDelay;
  }

  public int getMapLockedByMODelay() {
    return mapLockedByMODelay;
  }

  public void setMapLockedByMODelay(int mapLockedByMODelay) {
    this.mapLockedByMODelay = mapLockedByMODelay;
  }

  public int getMapMOLockTimeout() {
    return mapMOLockTimeout;
  }

  public void setMapMOLockTimeout(int mapMOLockTimeout) {
    this.mapMOLockTimeout = mapMOLockTimeout;
  }

  public boolean isMapAllowCallBarred() {
    return mapAllowCallBarred;
  }

  public void setMapAllowCallBarred(boolean mapAllowCallBarred) {
    this.mapAllowCallBarred = mapAllowCallBarred;
  }

  public boolean isMapUssdV1Enabled() {
    return mapUssdV1Enabled;
  }

  public void setMapUssdV1Enabled(boolean mapUssdV1Enabled) {
    this.mapUssdV1Enabled = mapUssdV1Enabled;
  }

  public boolean isMapUssdV1UseOrigEntityNumber() {
    return mapUssdV1UseOrigEntityNumber;
  }

  public void setMapUssdV1UseOrigEntityNumber(boolean mapUssdV1UseOrigEntityNumber) {
    this.mapUssdV1UseOrigEntityNumber = mapUssdV1UseOrigEntityNumber;
  }

  public int getMapIOTasksCount() {
    return mapIOTasksCount;
  }

  public void setMapIOTasksCount(int mapIOTasksCount) {
    this.mapIOTasksCount = mapIOTasksCount;
  }

  public int getTrafficShapeTimeFrame() {
    return trafficShapeTimeFrame;
  }

  public void setTrafficShapeTimeFrame(int trafficShapeTimeFrame) {
    this.trafficShapeTimeFrame = trafficShapeTimeFrame;
  }

  public int getTrafficStatTimeFrame() {
    return trafficStatTimeFrame;
  }

  public void setTrafficStatTimeFrame(int trafficStatTimeFrame) {
    this.trafficStatTimeFrame = trafficStatTimeFrame;
  }

  public int getMsArchiveInterval() {
    return msArchiveInterval;
  }

  public void setMsArchiveInterval(int msArchiveInterval) {
    this.msArchiveInterval = msArchiveInterval;
  }

  public int getLsMaxStoreSize() {
    return lsMaxStoreSize;
  }

  public void setLsMaxStoreSize(int lsMaxStoreSize) {
    this.lsMaxStoreSize = lsMaxStoreSize;
  }

  public int getLsMinRollTime() {
    return lsMinRollTime;
  }

  public void setLsMinRollTime(int lsMinRollTime) {
    this.lsMinRollTime = lsMinRollTime;
  }

  public int getSmppReadTimeout() {
    return smppReadTimeout;
  }

  public void setSmppReadTimeout(int smppReadTimeout) {
    this.smppReadTimeout = smppReadTimeout;
  }

  public int getSmppInactivityTime() {
    return smppInactivityTime;
  }

  public void setSmppInactivityTime(int smppInactivityTime) {
    this.smppInactivityTime = smppInactivityTime;
  }

  public int getSmppInactivityTimeOut() {
    return smppInactivityTimeOut;
  }

  public void setSmppInactivityTimeOut(int smppInactivityTimeOut) {
    this.smppInactivityTimeOut = smppInactivityTimeOut;
  }

  public int getSmppBindTimeout() {
    return smppBindTimeout;
  }

  public void setSmppBindTimeout(int smppBindTimeout) {
    this.smppBindTimeout = smppBindTimeout;
  }

  public int getSmppDefaultConnectionsLimit() {
    return smppDefaultConnectionsLimit;
  }

  public void setSmppDefaultConnectionsLimit(int smppDefaultConnectionsLimit) {
    this.smppDefaultConnectionsLimit = smppDefaultConnectionsLimit;
  }

  public String getProfilerSystemId() {
    return profilerSystemId;
  }

  public void setProfilerSystemId(String profilerSystemId) {
    this.profilerSystemId = profilerSystemId;
  }

  public String getProfilerServiceType() {
    return profilerServiceType;
  }

  public void setProfilerServiceType(String profilerServiceType) {
    this.profilerServiceType = profilerServiceType;
  }

  public int getProfilerProtocolId() {
    return profilerProtocolId;
  }

  public void setProfilerProtocolId(int profilerProtocolId) {
    this.profilerProtocolId = profilerProtocolId;
  }

  public String getProfilerStoreFile() {
    return profilerStoreFile;
  }

  public void setProfilerStoreFile(String profilerStoreFile) {
    this.profilerStoreFile = profilerStoreFile;
  }

  public int getProfilerReportNone() {
    return profilerReportNone;
  }

  public void setProfilerReportNone(int profilerReportNone) {
    this.profilerReportNone = profilerReportNone;
  }

  public int getProfilerReportFull() {
    return profilerReportFull;
  }

  public void setProfilerReportFull(int profilerReportFull) {
    this.profilerReportFull = profilerReportFull;
  }

  public int getProfilerReportFinal() {
    return profilerReportFinal;
  }

  public void setProfilerReportFinal(int profilerReportFinal) {
    this.profilerReportFinal = profilerReportFinal;
  }

  public int getProfilerLocaleRU() {
    return profilerLocaleRU;
  }

  public void setProfilerLocaleRU(int profilerLocaleRU) {
    this.profilerLocaleRU = profilerLocaleRU;
  }

  public int getProfilerLocaleEN() {
    return profilerLocaleEN;
  }

  public void setProfilerLocaleEN(int profilerLocaleEN) {
    this.profilerLocaleEN = profilerLocaleEN;
  }

  public int getProfilerDefault() {
    return profilerDefault;
  }

  public void setProfilerDefault(int profilerDefault) {
    this.profilerDefault = profilerDefault;
  }

  public int getProfilerUCS2() {
    return profilerUCS2;
  }

  public void setProfilerUCS2(int profilerUCS2) {
    this.profilerUCS2 = profilerUCS2;
  }

  public int getProfilerHide() {
    return profilerHide;
  }

  public void setProfilerHide(int profilerHide) {
    this.profilerHide = profilerHide;
  }

  public int getProfilerUnhide() {
    return profilerUnhide;
  }

  public void setProfilerUnhide(int profilerUnhide) {
    this.profilerUnhide = profilerUnhide;
  }

  public int getProfilerUSSD7BitOn() {
    return profilerUSSD7BitOn;
  }

  public void setProfilerUSSD7BitOn(int profilerUSSD7BitOn) {
    this.profilerUSSD7BitOn = profilerUSSD7BitOn;
  }

  public int getProfilerUSSD7BitOff() {
    return profilerUSSD7BitOff;
  }

  public void setProfilerUSSD7BitOff(int profilerUSSD7BitOff) {
    this.profilerUSSD7BitOff = profilerUSSD7BitOff;
  }

  public int getProfilerDivertOn() {
    return profilerDivertOn;
  }

  public void setProfilerDivertOn(int profilerDivertOn) {
    this.profilerDivertOn = profilerDivertOn;
  }

  public int getProfilerDivertOff() {
    return profilerDivertOff;
  }

  public void setProfilerDivertOff(int profilerDivertOff) {
    this.profilerDivertOff = profilerDivertOff;
  }

  public int getProfilerDivertAbsentOn() {
    return profilerDivertAbsentOn;
  }

  public void setProfilerDivertAbsentOn(int profilerDivertAbsentOn) {
    this.profilerDivertAbsentOn = profilerDivertAbsentOn;
  }

  public int getProfilerDivertAbsentOff() {
    return profilerDivertAbsentOff;
  }

  public void setProfilerDivertAbsentOff(int profilerDivertAbsentOff) {
    this.profilerDivertAbsentOff = profilerDivertAbsentOff;
  }

  public int getProfilerDivertBlockedOn() {
    return profilerDivertBlockedOn;
  }

  public void setProfilerDivertBlockedOn(int profilerDivertBlockedOn) {
    this.profilerDivertBlockedOn = profilerDivertBlockedOn;
  }

  public int getProfilerDivertBlockedOff() {
    return profilerDivertBlockedOff;
  }

  public void setProfilerDivertBlockedOff(int profilerDivertBlockedOff) {
    this.profilerDivertBlockedOff = profilerDivertBlockedOff;
  }

  public int getProfilerDivertBarredOn() {
    return profilerDivertBarredOn;
  }

  public void setProfilerDivertBarredOn(int profilerDivertBarredOn) {
    this.profilerDivertBarredOn = profilerDivertBarredOn;
  }

  public int getProfilerDivertBarredOff() {
    return profilerDivertBarredOff;
  }

  public void setProfilerDivertBarredOff(int profilerDivertBarredOff) {
    this.profilerDivertBarredOff = profilerDivertBarredOff;
  }

  public int getProfilerDivertCapacityOn() {
    return profilerDivertCapacityOn;
  }

  public void setProfilerDivertCapacityOn(int profilerDivertCapacityOn) {
    this.profilerDivertCapacityOn = profilerDivertCapacityOn;
  }

  public int getProfilerDivertCapacityOff() {
    return profilerDivertCapacityOff;
  }

  public void setProfilerDivertCapacityOff(int profilerDivertCapacityOff) {
    this.profilerDivertCapacityOff = profilerDivertCapacityOff;
  }

  public int getProfilerDivertStatus() {
    return profilerDivertStatus;
  }

  public void setProfilerDivertStatus(int profilerDivertStatus) {
    this.profilerDivertStatus = profilerDivertStatus;
  }

  public int getProfilerDivertTo() {
    return profilerDivertTo;
  }

  public void setProfilerDivertTo(int profilerDivertTo) {
    this.profilerDivertTo = profilerDivertTo;
  }

  public int getProfilerConcatOn() {
    return profilerConcatOn;
  }

  public void setProfilerConcatOn(int profilerConcatOn) {
    this.profilerConcatOn = profilerConcatOn;
  }

  public int getProfilerConcatOff() {
    return profilerConcatOff;
  }

  public void setProfilerConcatOff(int profilerConcatOff) {
    this.profilerConcatOff = profilerConcatOff;
  }

  public int getProfilerTranslitOn() {
    return profilerTranslitOn;
  }

  public void setProfilerTranslitOn(int profilerTranslitOn) {
    this.profilerTranslitOn = profilerTranslitOn;
  }

  public int getProfilerTranslitOff() {
    return profilerTranslitOff;
  }

  public void setProfilerTranslitOff(int profilerTranslitOff) {
    this.profilerTranslitOff = profilerTranslitOff;
  }

  public String getAbInfoSystemId() {
    return abInfoSystemId;
  }

  public void setAbInfoSystemId(String abInfoSystemId) {
    this.abInfoSystemId = abInfoSystemId;
  }

  public String getAbInfoServiceType() {
    return abInfoServiceType;
  }

  public void setAbInfoServiceType(String abInfoServiceType) {
    this.abInfoServiceType = abInfoServiceType;
  }

  public int getAbInfoProtocolId() {
    return abInfoProtocolId;
  }

  public void setAbInfoProtocolId(int abInfoProtocolId) {
    this.abInfoProtocolId = abInfoProtocolId;
  }

  public String getAbInfoMobileAccessAddress() {
    return abInfoMobileAccessAddress;
  }

  public void setAbInfoMobileAccessAddress(String abInfoMobileAccessAddress) {
    this.abInfoMobileAccessAddress = abInfoMobileAccessAddress;
  }

  public String getAbInfoSmppAccessAddress() {
    return abInfoSmppAccessAddress;
  }

  public void setAbInfoSmppAccessAddress(String abInfoSmppAccessAddress) {
    this.abInfoSmppAccessAddress = abInfoSmppAccessAddress;
  }

  public boolean isMscAutomaticRegistration() {
    return mscAutomaticRegistration;
  }

  public void setMscAutomaticRegistration(boolean mscAutomaticRegistration) {
    this.mscAutomaticRegistration = mscAutomaticRegistration;
  }

  public int getMscFailureLimit() {
    return mscFailureLimit;
  }

  public void setMscFailureLimit(int mscFailureLimit) {
    this.mscFailureLimit = mscFailureLimit;
  }

  public String getMscStoreFile() {
    return mscStoreFile;
  }

  public void setMscStoreFile(String mscStoreFile) {
    this.mscStoreFile = mscStoreFile;
  }

  public int getMscSingleAttemptTimeout() {
    return mscSingleAttemptTimeout;
  }

  public void setMscSingleAttemptTimeout(int mscSingleAttemptTimeout) {
    this.mscSingleAttemptTimeout = mscSingleAttemptTimeout;
  }

  public String getAclStoreDir() {
    return aclStoreDir;
  }

  public void setAclStoreDir(String aclStoreDir) {
    this.aclStoreDir = aclStoreDir;
  }

  public int getAclPreCreateSize() {
    return aclPreCreateSize;
  }

  public void setAclPreCreateSize(int aclPreCreateSize) {
    this.aclPreCreateSize = aclPreCreateSize;
  }

  public String getInmanHost() {
    return inmanHost;
  }

  public void setInmanHost(String inmanHost) {
    this.inmanHost = inmanHost;
  }

  public int getInmanPort() {
    return inmanPort;
  }

  public void setInmanPort(int inmanPort) {
    this.inmanPort = inmanPort;
  }

  public String getInmanChargingPeer2peer() {
    return inmanChargingPeer2peer;
  }

  public void setInmanChargingPeer2peer(String inmanChargingPeer2peer) {
    this.inmanChargingPeer2peer = inmanChargingPeer2peer;
  }

  public String getInmanChargingOther() {
    return inmanChargingOther;
  }

  public void setInmanChargingOther(String inmanChargingOther) {
    this.inmanChargingOther = inmanChargingOther;
  }

  public String getAliasStoreFile() {
    return aliasStoreFile;
  }

  public void setAliasStoreFile(String aliasStoreFile) {
    this.aliasStoreFile = aliasStoreFile;
  }

  public String getSnmpCsvFileDir() {
    return snmpCsvFileDir;
  }

  public void setSnmpCsvFileDir(String snmpCsvFileDir) {
    this.snmpCsvFileDir = snmpCsvFileDir;
  }

  public int getSnmpCsvFileRollInterval() {
    return snmpCsvFileRollInterval;
  }

  public void setSnmpCsvFileRollInterval(int snmpCsvFileRollInterval) {
    this.snmpCsvFileRollInterval = snmpCsvFileRollInterval;
  }

  public int getSnmpCacheTimeout() {
    return snmpCacheTimeout;
  }

  public void setSnmpCacheTimeout(int snmpCacheTimeout) {
    this.snmpCacheTimeout = snmpCacheTimeout;
  }

  public Map<String, String> getDirectives() {
    return directives;
  }

  public void setDirectives(Map<String, String> directives) {
    this.directives = directives;
  }
}
