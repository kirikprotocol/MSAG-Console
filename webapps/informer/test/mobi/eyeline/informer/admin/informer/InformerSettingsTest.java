package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class InformerSettingsTest {

  private InformerSettings cs;

  @Test
  public void adminHost() throws AdminException{
    try {
      cs.setAdminHost("");
      cs.validate();
      assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAdminHost(null);
      cs.validate();
      assertTrue(false);
    } catch (AdminException e) {}

    cs.setAdminHost("host");
    assertEquals("host", cs.getAdminHost());
  }

  @Test
  public void persHost() throws AdminException{
    try {
      cs.setPersHost("");
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setPersHost(null);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setPersHost("host");
    assertEquals("host", cs.getPersHost());
  }

  @Test
  public void adminPort() throws AdminException{
    try {
      cs.setAdminPort(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAdminPort(65536);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setAdminPort(1);
    assertEquals(1, cs.getAdminPort());
  }

  @Test
  public void persPort() throws AdminException{
    try {
      cs.setPersSyncPort(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setPersSyncPort(65536);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setPersSyncPort(1);
    assertEquals(1, cs.getPersSyncPort());
  }

  @Test
  public void dcpHost() throws AdminException{
    try {
      cs.setDcpHost("");
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDcpHost(null);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setDcpHost("host");
    assertEquals("host", cs.getDcpHost());
  }

  @Test
  public void dcpPort() throws AdminException{
    try {
      cs.setDcpPort(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDcpPort(65536);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setDcpPort(1);
    assertEquals(1, cs.getDcpPort());
  }

  @Test
  public void inputMinQueueSize() throws AdminException{
    try {
      cs.setInputMinQueueSize(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setInputMinQueueSize(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setInputMinQueueSize(1);
    assertEquals(1, cs.getInputMinQueueSize());

  }

  @Test
  public void resendIOThreadCount() throws AdminException{
    try {
      cs.setResendIOThreadCount(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setResendIOThreadCount(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setResendIOThreadCount(1);
    assertEquals(1, cs.getResendIOThreadCount());

  }
  @Test
  public void inputTransferThreadCount() throws AdminException{
    try {
      cs.setInputTransferThreadCount(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setInputTransferThreadCount(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setInputTransferThreadCount(1);
    assertEquals(1, cs.getInputTransferThreadCount());

  }

  @Test
  public void inputReceiptExtraWaitTime() throws AdminException{
    try {
      cs.setReceiptExtraWaitTime(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setReceiptExtraWaitTime(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setReceiptExtraWaitTime(1);
    assertEquals(1, cs.getReceiptExtraWaitTime());
  }

  @Test
  public void inputInputTransferChunkSizee() throws AdminException{
    try {
      cs.setInputTransferChunkSize(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setInputTransferChunkSize(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setInputTransferChunkSize(1);
    assertEquals(1, cs.getInputTransferChunkSize());
  }

  @Test
  public void skippedIdsChunkSize() throws AdminException{
    try {
      cs.setSkippedIdsChunkSize(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setSkippedIdsChunkSize(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setSkippedIdsChunkSize(1);
    assertEquals(1, cs.getSkippedIdsChunkSize());
  }

  @Test
  public void inputJournalRollingPeriod() throws AdminException{
    try {
      cs.setInputJournalRollingPeriod(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setInputJournalRollingPeriod(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setInputJournalRollingPeriod(1);
    assertEquals(1, cs.getInputJournalRollingPeriod());
  }

  @Test
  public void unrespondedMessagesMax() throws AdminException{
    try {
      cs.setUnrespondedMessagesMax(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setUnrespondedMessagesMax(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setUnrespondedMessagesMax(1);
    assertEquals(1, cs.getUnrespondedMessagesMax());
  }

  @Test
  public void skippedIdsMinCacheSize() throws AdminException{
    try {
      cs.setSkippedIdsMinCacheSize(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setSkippedIdsMinCacheSize(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setSkippedIdsMinCacheSize(1);
    assertEquals(1, cs.getSkippedIdsMinCacheSize());
  }

  @Test
  public void slicedMessageSize() throws AdminException{
    try {
      cs.setSlicedMessageSize(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setSlicedMessageSize(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setSlicedMessageSize(1);
    assertEquals(1, cs.getSlicedMessageSize());
  }

  @Test
  public void almRequestTimeout() throws AdminException{
    try {
      cs.setAlmRequestTimeout(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAlmRequestTimeout(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setAlmRequestTimeout(1);
    assertEquals(1, cs.getAlmRequestTimeout());
  }

  @Test
  public void operationalJournalRollingPeriod() throws AdminException{
    try {
      cs.setOperationalJournalRollingPeriod(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setOperationalJournalRollingPeriod(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setOperationalJournalRollingPeriod(1);
    assertEquals(1, cs.getOperationalJournalRollingPeriod());
  }

  @Test
  public void dcpHandlers() throws AdminException{
    try {
      cs.setDcpHandlers(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setDcpHandlers(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setDcpHandlers(1);
    assertEquals(1, cs.getDcpHandlers());
  }

  @Test
  public void svcType() throws AdminException{
    try {
      cs.setSvcType(null);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setSvcType("");
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setSvcType("s1");
    assertEquals("s1", cs.getSvcType());
  }

  @Test
  public void adminHandlers() throws AdminException{
    try {
      cs.setAdminHandlers(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setAdminHandlers(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setAdminHandlers(1);
    assertEquals(1, cs.getAdminHandlers());
  }

  @Test
  public void retryMinTimeToLive() throws AdminException{
    try {
      cs.setRetryMinTimeToLive(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setRetryMinTimeToLive(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setRetryMinTimeToLive(1);
    assertEquals(1, cs.getRetryMinTimeToLive());
  }

  @Test
  public void responseWaitTim() throws AdminException{
    try {
      cs.setResponseWaitTime(-1);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}
    try {
      cs.setResponseWaitTime(0);
      cs.validate(); assertTrue(false);
    } catch (AdminException e) {}

    cs.setResponseWaitTime(1);
    assertEquals(1, cs.getResponseWaitTime());
  }

  @Test
  public void cloneAndValidateSettings() throws AdminException { 

    cs.validate();

    InformerSettings cs1 = cs.cloneSettings();

    cs1.validate();

    assertTrue(cs != cs1);
    assertEquals(cs.getAdminHost(), cs1.getAdminHost());
    assertEquals(cs.getAdminPort(), cs1.getAdminPort());
    assertEquals(cs.getDcpHost(), cs1.getDcpHost());
    assertEquals(cs.getDcpPort(), cs1.getDcpPort());
    assertEquals(cs.getPersHost(), cs1.getPersHost());
    assertEquals(cs.getPersSyncPort(), cs1.getPersSyncPort());
    assertEquals(cs.getStatDir(), cs1.getStatDir());
    assertEquals(cs.getStoreDir(), cs1.getStoreDir());
  }


  @Before
  public void before() {
    cs = new InformerSettings();
    cs.setAdminHost("adminHost");
    cs.setAdminPort(1);
    cs.setDcpHost("dcpHost");
    cs.setDcpPort(2);
    cs.setPersHost("persHost");
    cs.setPersSyncPort(3);
    cs.setStatDir("statDir");
    cs.setStoreDir("storeDir");

    cs.setInputMinQueueSize(2);
    cs.setResendIOThreadCount(1);
    cs.setInputTransferThreadCount(43);
    cs.setReceiptExtraWaitTime(634);
    cs.setInputTransferChunkSize(234);
    cs.setSkippedIdsChunkSize(64);
    cs.setInputJournalRollingPeriod(253);
    cs.setUnrespondedMessagesMax(24);
    cs.setSkippedIdsMinCacheSize(25);
    cs.setSlicedMessageSize(34);
    cs.setAlmRequestTimeout(64);
    cs.setOperationalJournalRollingPeriod(74);
    cs.setDcpHandlers(23);
    cs.setSvcType("2323werczxcf213");
    cs.setAdminHandlers(463);
    cs.setRetryMinTimeToLive(763);
    cs.setResponseWaitTime(62);
  }
}
