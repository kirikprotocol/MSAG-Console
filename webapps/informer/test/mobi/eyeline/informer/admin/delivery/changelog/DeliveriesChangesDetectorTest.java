package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.Functions;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.TimeZone;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 11:41:45
 */
public class DeliveriesChangesDetectorTest implements DeliveryChangeListener {

  static File dstStatDir;
  static TestDeliveryChangesDetector producer;
  List stateEventChanges = new ArrayList();

  private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  @BeforeClass
  public static void init() throws Exception{
    FileSystem fileSystem = FileSystem.getFSForSingleInst();
    dstStatDir = TestUtils.createRandomDir(".delivery.changelog.backup");
    TestUtils.exportResource(DeliveriesChangesDetectorTest.class.getResourceAsStream("201011120830.csv"), new File(dstStatDir, "201011120830.csv"));

    producer = new TestDeliveryChangesDetector(dstStatDir,fileSystem);
  }

  @AfterClass
  public static void shutdown() {
    if(dstStatDir != null) {
      TestUtils.recursiveDeleteFolder(dstStatDir);
    }
  }

  @Test
  public void testReadDirectory() throws InterruptedException, ParseException {
    producer.addListener(this);
    stateEventChanges.clear();
    producer.run();

    assertTrue(stateEventChanges.size()==5);

    List<String> initialNames = Arrays.asList((new File(dstStatDir,"processedFiles")).list());
    //check all files processed
    File[] files = dstStatDir.listFiles();
    for(File f : files) {
      assertTrue(f.isDirectory());
    }

    //check files moved
    List<String> processedNames = Arrays.asList((new File(dstStatDir,"processedFiles")).list());
    assertTrue(initialNames.equals(processedNames));


    //15,1,"a",1

    ChangeDeliveryStatusEvent n = (ChangeDeliveryStatusEvent)stateEventChanges.get(0);
    assertTrue(n.getEventDate().equals(Functions.convertTime(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083015"), STAT_TIMEZONE, LOCAL_TIMEZONE)));
    assertTrue(n.getDeliveryId()==1);
    assertTrue(n.getUserId().equals("a"));
    assertTrue(n.getStatus() == DeliveryStatus.Active);

    //18,1,"a",0,1,D,0,+79130000001,userdata1
    ChangeMessageStateEvent m = (ChangeMessageStateEvent) stateEventChanges.get(1);
    assertTrue(m.getEventDate().equals(Functions.convertTime(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083018"), STAT_TIMEZONE, LOCAL_TIMEZONE)));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getMessageState() == MessageState.Delivered);
    assertTrue(m.getMessageId()==1);
    assertTrue(m.getSmppStatus()==0);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000001"));
    assertEquals("1", m.getProperties().getProperty("userdata1"));
    assertEquals(3, m.getNsms());

    //18,1,"a",0,2,F,1,+79130000002,userdata2
    m = (ChangeMessageStateEvent) stateEventChanges.get(2);
    assertTrue(m.getEventDate().equals(Functions.convertTime(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083018"), STAT_TIMEZONE, LOCAL_TIMEZONE)));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getMessageId()==2);
    assertTrue(m.getMessageState()==MessageState.Failed);
    assertTrue(m.getSmppStatus()==1);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000002"));
    assertEquals("2", m.getProperties().getProperty("userdata2"));
    assertEquals(4, m.getNsms());

    //20,1,"a",0,3,E,2,+79130000003
    m = (ChangeMessageStateEvent) stateEventChanges.get(3);
    assertTrue(m.getEventDate().equals(Functions.convertTime(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083020"), STAT_TIMEZONE, LOCAL_TIMEZONE)));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getMessageId()==3);
    assertTrue(m.getMessageState()== MessageState.Expired);
    assertTrue(m.getSmppStatus()==2);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000003"));
    assertEquals(0, m.getProperties().size());
    assertEquals(5, m.getNsms());

    //35,2,"b",2
    n = (ChangeDeliveryStatusEvent)stateEventChanges.get(4);
    assertTrue(n.getEventDate().equals(Functions.convertTime(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083035"), STAT_TIMEZONE, LOCAL_TIMEZONE)));
    assertTrue(n.getDeliveryId()==2);
    assertTrue(n.getUserId().equals("b"));
    assertTrue(n.getStatus() == DeliveryStatus.Finished);

  }

  public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
     stateEventChanges.add(e);
  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
     stateEventChanges.add(e);
  }
}
