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
      
  public boolean isPvssEnabledBL() {
    return pvssEnabledBL;
  }

  public void setPvssEnabledBL(boolean pvssEnabledBL) {
    this.pvssEnabledBL = pvssEnabledBL;
  }

  public int getResendIOThreadCount() {
    return resendIOThreadCount;
  }

  public void setResendIOThreadCount(int resendIOThreadCount) throws AdminException{
    vh.checkPositive("resendIOThreadCount", resendIOThreadCount);
    this.resendIOThreadCount = resendIOThreadCount;
  }

  public int getInputMinQueueSize() {
    return inputMinQueueSize;
  }

  public void setInputMinQueueSize(int inputMinQueueSize) throws AdminException{
    vh.checkPositive("inputMinQueueSize", inputMinQueueSize);
    this.inputMinQueueSize = inputMinQueueSize;
  }

  public int getInputTransferThreadCount() {
    return inputTransferThreadCount;
  }

  public void setInputTransferThreadCount(int inputTransferThreadCount) throws AdminException{
    vh.checkPositive("inputTransferThreadCount", inputTransferThreadCount);
    this.inputTransferThreadCount = inputTransferThreadCount;
  }

  public int getProtocolId() {
    return protocolId;
  }

  public void setProtocolId(int protocolId) throws AdminException{
    vh.checkGreaterOrEqualsTo("protocolId", protocolId, 0);
    this.protocolId = protocolId;
  }

  public int getReceiptExtraWaitTime() {
    return receiptExtraWaitTime;
  }

  public void setReceiptExtraWaitTime(int receiptExtraWaitTime) throws AdminException{
    vh.checkPositive("receiptExtraWaitTime", receiptExtraWaitTime);
    this.receiptExtraWaitTime = receiptExtraWaitTime;
  }

  public int getInputTransferChunkSize() {
    return inputTransferChunkSize;
  }

  public void setInputTransferChunkSize(int inputTransferChunkSize) throws AdminException{
    vh.checkPositive("inputTransferChunkSize", inputTransferChunkSize);
    this.inputTransferChunkSize = inputTransferChunkSize;
  }

  public int getSkippedIdsChunkSize() {
    return skippedIdsChunkSize;
  }

  public void setSkippedIdsChunkSize(int skippedIdsChunkSize) throws AdminException{
    vh.checkPositive("skippedIdsChunkSize", skippedIdsChunkSize);
    this.skippedIdsChunkSize = skippedIdsChunkSize;
  }

  public int getInputJournalRollingPeriod() {
    return inputJournalRollingPeriod;
  }

  public void setInputJournalRollingPeriod(int inputJournalRollingPeriod) throws AdminException{
    vh.checkPositive("inputJournalRollingPeriod", inputJournalRollingPeriod);
    this.inputJournalRollingPeriod = inputJournalRollingPeriod;
  }

  public int getUnrespondedMessagesMax() {
    return unrespondedMessagesMax;
  }

  public void setUnrespondedMessagesMax(int unrespondedMessagesMax) throws AdminException{
    vh.checkPositive("unrespondedMessagesMax", unrespondedMessagesMax);
    this.unrespondedMessagesMax = unrespondedMessagesMax;
  }

  public int getSkippedIdsMinCacheSize() {
    return skippedIdsMinCacheSize;
  }

  public void setSkippedIdsMinCacheSize(int skippedIdsMinCacheSize) throws AdminException{
    vh.checkPositive("skippedIdsMinCacheSize", skippedIdsMinCacheSize);
    this.skippedIdsMinCacheSize = skippedIdsMinCacheSize;
  }

  public int getSlicedMessageSize() {
    return slicedMessageSize;
  }

  public void setSlicedMessageSize(int slicedMessageSize) throws AdminException{
    vh.checkPositive("slicedMessageSize", slicedMessageSize);
    this.slicedMessageSize = slicedMessageSize;
  }

  public int getAlmRequestTimeout() {
    return almRequestTimeout;
  }

  public void setAlmRequestTimeout(int almRequestTimeout) throws AdminException{
    vh.checkPositive("almRequestTimeout", almRequestTimeout);
    this.almRequestTimeout = almRequestTimeout;
  }

  public int getOperationalJournalRollingPeriod() {
    return operationalJournalRollingPeriod;
  }

  public void setOperationalJournalRollingPeriod(int operationalJournalRollingPeriod) throws AdminException{
    vh.checkPositive("operationalJournalRollingPeriod", operationalJournalRollingPeriod);
    this.operationalJournalRollingPeriod = operationalJournalRollingPeriod;
  }

  public int getDcpHandlers() {
    return dcpHandlers;
  }

  public void setDcpHandlers(int dcpHandlers) throws AdminException{
    vh.checkPositive("dcpHandlers", dcpHandlers);
    this.dcpHandlers = dcpHandlers;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) throws AdminException{
    vh.checkNotEmpty("svcType", svcType);
    this.svcType = svcType;
  }

  public int getAdminHandlers() {
    return adminHandlers;
  }

  public void setAdminHandlers(int adminHandlers) throws AdminException{
    vh.checkPositive("adminHandlers", adminHandlers);
    this.adminHandlers = adminHandlers;
  }

  public int getRetryMinTimeToLive() {
    return retryMinTimeToLive;
  }

  public void setRetryMinTimeToLive(int retryMinTimeToLive) throws AdminException{
    vh.checkPositive("retryMinTimeToLive", retryMinTimeToLive);
    this.retryMinTimeToLive = retryMinTimeToLive;
  }

  public int getResponseWaitTime() {
    return responseWaitTime;
  }

  public void setResponseWaitTime(int responseWaitTime) throws AdminException{
    vh.checkPositive("responseWaitTime", responseWaitTime);
    this.responseWaitTime = responseWaitTime;
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String host) throws AdminException {
    vh.checkNotEmpty("adminHost", host);
    this.adminHost = host;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) throws AdminException {
    vh.checkPort("adminPort", adminPort);
    this.adminPort = adminPort;
  }

  public int getDcpPort() {
    return dcpPort;
  }

  public void setDcpPort(int port) throws AdminException {
    vh.checkPort("dcpPort", port);
    this.dcpPort = port;
  }

  public String getDcpHost() {
    return dcpHost;
  }

  public void setDcpHost(String dcpHost) throws AdminException {
    vh.checkNotEmpty("dcpHost", dcpHost);
    this.dcpHost = dcpHost;
  }

  public String getPersHost() {
    return persHost;
  }

  public void setPersHost(String persHost) throws AdminException {
    vh.checkNotEmpty("persHost", persHost);
    this.persHost = persHost;
  }

  public int getPersSyncPort() {
    return persSyncPort;
  }

  public void setPersSyncPort(int persSyncPort) throws AdminException {
    vh.checkPort("persSyncPort", persSyncPort);
    this.persSyncPort = persSyncPort;
  }

  public int getPersAsyncPort() {
    return persAsyncPort;
  }

  public void setPersAsyncPort(int persAsyncPort) throws AdminException {
    vh.checkPort("persAsyncPort", persAsyncPort);
    this.persAsyncPort = persAsyncPort;
  }

  public String getStatDir() {
    return statDir;
  }

  public void setStatDir(String statDir) throws AdminException {
    vh.checkNotEmpty("statDir", statDir);
    this.statDir = statDir;
  }

  public String getStoreDir() {
    return storeDir;
  }

  public void setStoreDir(String storeDir) throws AdminException {
    vh.checkNotEmpty("storeDir", storeDir);
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
