package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

/**
 * Настройки Informer
 *
 * @author Aleksandr Khalitov
 */
public class InformerSettings {

  private final ValidationHelper vh = new ValidationHelper(InformerSettings.class);

  private String adminHost;
  private int adminPort;

  private String dcpHost;
  private int dcpPort;

  private String persHost;
  private int persSyncPort;
  private int persAsyncPort;

  private String statDir;
  private String storeDir;

  // additional

  private int resendIOThreadCount;
  private int inputMinQueueSize;
  private int inputTransferThreadCount;
  private int protocolId;
  private int receiptExtraWaitTime;
  private int inputTransferChunkSize;
  private int skippedIdsChunkSize;
  private int inputJournalRollingPeriod;
  private int unrespondedMessagesMax;
  private int skippedIdsMinCacheSize;
  private int slicedMessageSize;
  private int almRequestTimeout;
  private int operationalJournalRollingPeriod;
  private int dcpHandlers;
  private String svcType;
  private int adminHandlers;
  private int retryMinTimeToLive;
  private int responseWaitTime;
  private boolean pvssEnabledBL;

  private String archiveHost;
  private int archivePort;

  void validate() throws AdminException {
    vh.checkPositive("resendIOThreadCount", resendIOThreadCount);
    vh.checkPositive("inputMinQueueSize", inputMinQueueSize);
    vh.checkPositive("inputTransferThreadCount", inputTransferThreadCount);
    vh.checkGreaterOrEqualsTo("protocolId", protocolId, 0);
    vh.checkPositive("receiptExtraWaitTime", receiptExtraWaitTime);
    vh.checkPositive("inputTransferChunkSize", inputTransferChunkSize);
    vh.checkPositive("skippedIdsChunkSize", skippedIdsChunkSize);
    vh.checkPositive("inputJournalRollingPeriod", inputJournalRollingPeriod);
    vh.checkPositive("unrespondedMessagesMax", unrespondedMessagesMax);
    vh.checkPositive("skippedIdsMinCacheSize", skippedIdsMinCacheSize);
    vh.checkPositive("slicedMessageSize", slicedMessageSize);
    vh.checkPositive("almRequestTimeout", almRequestTimeout);
    vh.checkPositive("operationalJournalRollingPeriod", operationalJournalRollingPeriod);
    vh.checkPositive("dcpHandlers", dcpHandlers);
    vh.checkNotEmpty("svcType", svcType);
    vh.checkPositive("adminHandlers", adminHandlers);
    vh.checkPositive("retryMinTimeToLive", retryMinTimeToLive);
    vh.checkPositive("responseWaitTime", responseWaitTime);
    vh.checkNotEmpty("adminHost", adminHost);
    vh.checkPort("adminPort", adminPort);
    vh.checkPort("dcpPort", dcpPort);
    vh.checkNotEmpty("dcpHost", dcpHost);
    vh.checkNotEmpty("persHost", persHost);
    vh.checkPort("persSyncPort", persSyncPort);
    vh.checkPort("persAsyncPort", persAsyncPort);
    vh.checkNotEmpty("statDir", statDir);
    vh.checkNotEmpty("storeDir", storeDir);
  }

  public boolean isPvssEnabledBL() {
    return pvssEnabledBL;
  }

  public void setPvssEnabledBL(boolean pvssEnabledBL) {
    this.pvssEnabledBL = pvssEnabledBL;
  }

  public int getResendIOThreadCount() {
    return resendIOThreadCount;
  }

  public void setResendIOThreadCount(int resendIOThreadCount){
    this.resendIOThreadCount = resendIOThreadCount;
  }

  public int getInputMinQueueSize() {
    return inputMinQueueSize;
  }

  public void setInputMinQueueSize(int inputMinQueueSize) {
    this.inputMinQueueSize = inputMinQueueSize;
  }

  public int getInputTransferThreadCount() {
    return inputTransferThreadCount;
  }

  public void setInputTransferThreadCount(int inputTransferThreadCount) {
    this.inputTransferThreadCount = inputTransferThreadCount;
  }

  public int getProtocolId() {
    return protocolId;
  }

  public void setProtocolId(int protocolId){
    this.protocolId = protocolId;
  }

  public int getReceiptExtraWaitTime() {
    return receiptExtraWaitTime;
  }

  public void setReceiptExtraWaitTime(int receiptExtraWaitTime){
    this.receiptExtraWaitTime = receiptExtraWaitTime;
  }

  public int getInputTransferChunkSize() {
    return inputTransferChunkSize;
  }

  public void setInputTransferChunkSize(int inputTransferChunkSize) {
    this.inputTransferChunkSize = inputTransferChunkSize;
  }

  public int getSkippedIdsChunkSize() {
    return skippedIdsChunkSize;
  }

  public void setSkippedIdsChunkSize(int skippedIdsChunkSize) {
    this.skippedIdsChunkSize = skippedIdsChunkSize;
  }

  public int getInputJournalRollingPeriod() {
    return inputJournalRollingPeriod;
  }

  public void setInputJournalRollingPeriod(int inputJournalRollingPeriod) {
    this.inputJournalRollingPeriod = inputJournalRollingPeriod;
  }

  public int getUnrespondedMessagesMax() {
    return unrespondedMessagesMax;
  }

  public void setUnrespondedMessagesMax(int unrespondedMessagesMax) {
    this.unrespondedMessagesMax = unrespondedMessagesMax;
  }

  public int getSkippedIdsMinCacheSize() {
    return skippedIdsMinCacheSize;
  }

  public void setSkippedIdsMinCacheSize(int skippedIdsMinCacheSize) {
    this.skippedIdsMinCacheSize = skippedIdsMinCacheSize;
  }

  public int getSlicedMessageSize() {
    return slicedMessageSize;
  }

  public void setSlicedMessageSize(int slicedMessageSize){
    this.slicedMessageSize = slicedMessageSize;
  }

  public int getAlmRequestTimeout() {
    return almRequestTimeout;
  }

  public void setAlmRequestTimeout(int almRequestTimeout) {
    this.almRequestTimeout = almRequestTimeout;
  }

  public int getOperationalJournalRollingPeriod() {
    return operationalJournalRollingPeriod;
  }

  public void setOperationalJournalRollingPeriod(int operationalJournalRollingPeriod) {
    this.operationalJournalRollingPeriod = operationalJournalRollingPeriod;
  }

  public int getDcpHandlers() {
    return dcpHandlers;
  }

  public void setDcpHandlers(int dcpHandlers) {
    this.dcpHandlers = dcpHandlers;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  public int getAdminHandlers() {
    return adminHandlers;
  }

  public void setAdminHandlers(int adminHandlers) {
    this.adminHandlers = adminHandlers;
  }

  public int getRetryMinTimeToLive() {
    return retryMinTimeToLive;
  }

  public void setRetryMinTimeToLive(int retryMinTimeToLive) {
    this.retryMinTimeToLive = retryMinTimeToLive;
  }

  public int getResponseWaitTime() {
    return responseWaitTime;
  }

  public void setResponseWaitTime(int responseWaitTime) {
    this.responseWaitTime = responseWaitTime;
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String host) {
    this.adminHost = host;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) {
    this.adminPort = adminPort;
  }

  public String getArchiveHost() {
    return archiveHost;
  }

  public void setArchiveHost(String archiveHost) {
    this.archiveHost = archiveHost;
  }

  public int getArchivePort() {
    return archivePort;
  }

  public void setArchivePort(int archivePort) {
    this.archivePort = archivePort;
  }

  public int getDcpPort() {
    return dcpPort;
  }

  public void setDcpPort(int port) {
    this.dcpPort = port;
  }

  public String getDcpHost() {
    return dcpHost;
  }

  public void setDcpHost(String dcpHost) {
    this.dcpHost = dcpHost;
  }

  public String getPersHost() {
    return persHost;
  }

  public void setPersHost(String persHost) {
    this.persHost = persHost;
  }

  public int getPersSyncPort() {
    return persSyncPort;
  }

  public void setPersSyncPort(int persSyncPort) {
    this.persSyncPort = persSyncPort;
  }

  public int getPersAsyncPort() {
    return persAsyncPort;
  }

  public void setPersAsyncPort(int persAsyncPort) {
    this.persAsyncPort = persAsyncPort;
  }

  public String getStatDir() {
    return statDir;
  }

  public void setStatDir(String statDir) {
    this.statDir = statDir;
  }

  public String getStoreDir() {
    return storeDir;
  }

  public void setStoreDir(String storeDir) {
    this.storeDir = storeDir;
  }

  /**
   * Копирует настройки
   *
   * @return копия настроек
   */
  public InformerSettings cloneSettings() {
    InformerSettings cs = new InformerSettings();
    cs.adminHost = adminHost;
    cs.adminPort = adminPort;
    cs.dcpHost = dcpHost;
    cs.dcpPort = dcpPort;
    cs.archiveHost = archiveHost;
    cs.archivePort = archivePort;
    cs.persHost = persHost;
    cs.persSyncPort = persSyncPort;
    cs.persAsyncPort = persAsyncPort;
    cs.statDir = statDir;
    cs.storeDir = storeDir;

    cs.resendIOThreadCount=resendIOThreadCount;
    cs.inputMinQueueSize=inputMinQueueSize;
    cs.inputTransferThreadCount=inputTransferThreadCount;
    cs.protocolId=protocolId;
    cs.receiptExtraWaitTime=receiptExtraWaitTime;
    cs.inputTransferChunkSize=inputTransferChunkSize;
    cs.skippedIdsChunkSize=skippedIdsChunkSize;
    cs.inputJournalRollingPeriod=inputJournalRollingPeriod;
    cs.unrespondedMessagesMax=unrespondedMessagesMax;
    cs.skippedIdsMinCacheSize=skippedIdsMinCacheSize;
    cs.slicedMessageSize=slicedMessageSize;
    cs.almRequestTimeout=almRequestTimeout;
    cs.operationalJournalRollingPeriod=operationalJournalRollingPeriod;
    cs.dcpHandlers=dcpHandlers;
    cs.svcType=svcType;
    cs.adminHandlers=adminHandlers;
    cs.retryMinTimeToLive=retryMinTimeToLive;
    cs.responseWaitTime=responseWaitTime;
    cs.pvssEnabledBL = pvssEnabledBL;

    return cs;
  }

}
