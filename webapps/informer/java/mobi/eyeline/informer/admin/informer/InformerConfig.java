package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */
class InformerConfig implements ManagedConfigFile<InformerSettings> {

  public InformerSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);

    InformerSettings settings = new InformerSettings();

    XmlConfigSection s = config.getSection("informer");
    settings.setAdminHost(s.getString("adminHost"));
    settings.setAdminPort(s.getInt("adminPort"));

    settings.setDcpHost(s.getString("dcpHost"));
    settings.setDcpPort(s.getInt("dcpPort"));

    settings.setStatDir(s.getString("statPath", null));
    settings.setStoreDir(s.getString("storePath"));

    XmlConfigSection s1 = s.getSection("finallog");
    settings.setWebFinalLogsDir(s1.getString("webapp"));
    settings.setSmppGWFinalLogsDir(s1.getString("dcpgw", null));

    settings.setResendIOThreadCount(s.getInt("resendIOThreadCount"));
    settings.setInputMinQueueSize(s.getInt("inputMinQueueSize"));
    settings.setInputTransferThreadCount(s.getInt("inputTransferThreadCount"));
    settings.setProtocolId(s.getInt("protocolId"));
    settings.setReceiptExtraWaitTime(s.getInt("receiptExtraWaitTime"));
    settings.setInputTransferChunkSize(s.getInt("inputTransferChunkSize"));
    settings.setSkippedIdsChunkSize(s.getInt("skippedIdsChunkSize"));
    settings.setInputJournalRollingPeriod(s.getInt("inputJournalRollingPeriod"));
    settings.setUnrespondedMessagesMax(s.getInt("unrespondedMessagesMax"));
    settings.setSkippedIdsMinCacheSize(s.getInt("skippedIdsMinCacheSize"));
    settings.setSlicedMessageSize(s.getInt("slicedMessageSize"));
    settings.setAlmRequestTimeout(s.getInt("almRequestTimeout"));
    settings.setOperationalJournalRollingPeriod(s.getInt("operationalJournalRollingPeriod"));
    settings.setDcpHandlers(s.getInt("dcpHandlers"));
    settings.setSvcType(s.getString("svcType"));
    settings.setAdminHandlers(s.getInt("adminHandlers"));
    settings.setRetryMinTimeToLive(s.getInt("retryMinTimeToLive"));
    settings.setResponseWaitTime(s.getInt("responseWaitTime"));

    s = config.getSection("pvss");
    settings.setPersHost(s.getString("host"));
    settings.setPersSyncPort(s.getInt("syncPort"));
    settings.setPersAsyncPort(s.getInt("asyncPort"));
    settings.setPvssEnabledBL(s.getBool("enabled"));

    return settings;
  }

  public void save(InputStream is, OutputStream os, InformerSettings settings) throws Exception {
    XmlConfig config = new XmlConfig(is);
    XmlConfigSection s = config.getOrCreateSection("informer");
    s.setString("adminHost", settings.getAdminHost());
    s.setInt("adminPort", settings.getAdminPort());
    s.setString("dcpHost", settings.getDcpHost());
    s.setInt("dcpPort", settings.getDcpPort());
    s.setString("statPath", settings.getStatDir());
    s.setString("storePath", settings.getStoreDir());

    XmlConfigSection s1 = s.getOrCreateSection("finallog");
    s1.clear();
    s1.setString("webapp",settings.getWebFinalLogsDir());
    if(settings.getSmppGWFinalLogsDir() != null) {
      s1.setString("dcpgw",settings.getSmppGWFinalLogsDir());
    }

    s.setInt("resendIOThreadCount", settings.getResendIOThreadCount());
    s.setInt("inputMinQueueSize", settings.getInputMinQueueSize());
    s.setInt("inputTransferThreadCount", settings.getInputTransferThreadCount());
    s.setInt("protocolId", settings.getProtocolId());
    s.setInt("receiptExtraWaitTime", settings.getReceiptExtraWaitTime());
    s.setInt("inputTransferChunkSize", settings.getInputTransferChunkSize());
    s.setInt("skippedIdsChunkSize", settings.getSkippedIdsChunkSize());
    s.setInt("inputJournalRollingPeriod", settings.getInputJournalRollingPeriod());
    s.setInt("unrespondedMessagesMax", settings.getUnrespondedMessagesMax());
    s.setInt("skippedIdsMinCacheSize", settings.getSkippedIdsMinCacheSize());
    s.setInt("slicedMessageSize", settings.getSlicedMessageSize());
    s.setInt("almRequestTimeout", settings.getAlmRequestTimeout());
    s.setInt("operationalJournalRollingPeriod", settings.getOperationalJournalRollingPeriod());
    s.setInt("dcpHandlers", settings.getDcpHandlers());
    s.setString("svcType", settings.getSvcType());
    s.setInt("adminHandlers", settings.getAdminHandlers());
    s.setInt("retryMinTimeToLive", settings.getRetryMinTimeToLive());
    s.setInt("responseWaitTime", settings.getResponseWaitTime());


    s = config.getOrCreateSection("pvss");
    s.setString("host", settings.getPersHost());
    s.setInt("syncPort", settings.getPersSyncPort());
    s.setInt("asyncPort", settings.getPersAsyncPort());
    s.setBool("enabled", settings.isPvssEnabledBL());

    config.save(os);
  }
}
