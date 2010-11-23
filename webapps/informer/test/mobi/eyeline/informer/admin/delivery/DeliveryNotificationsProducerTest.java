package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.notifications.DeliveryMessageNotification;
import mobi.eyeline.informer.admin.notifications.DeliveryNotification;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationType;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsListener;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.InputStream;
import java.net.URI;
import java.net.URL;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static junit.framework.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 11:41:45
 */
public class DeliveryNotificationsProducerTest implements DeliveryNotificationsListener {
  static File dstStatDir;
  static TestDeliveryNotificationsProducer producer;
  private static FileSystem fileSystem;
  List<DeliveryNotification> notifications = new ArrayList<DeliveryNotification>();

  @BeforeClass
  public static void init() throws Exception{
    fileSystem = FileSystem.getFSForSingleInst();
    dstStatDir = TestUtils.createRandomDir(".statuslogs.backup");



      URL u = TestDeliveryStatProvider.class.getResource("");
      URI uri = u.toURI();

      if("jar".equals(uri.getScheme())) {
        String jarPath = uri.getSchemeSpecificPart();
        String jarFileURI   = jarPath.substring(0,jarPath.indexOf("!/"));
        String jarEntryPathURI = jarPath.substring(jarPath.indexOf("!/")+2)+"statuslogs";
        InputStream is = null;
        try {
          is = fileSystem.getInputStream(new File(new URI(jarFileURI)));
          TestUtils.extractDirFromJar(is,jarEntryPathURI,dstStatDir,fileSystem);
        }
        finally {
          if(is!=null) is.close();
        }

      }
      else {
        File srcStatDir = new File(uri);
        srcStatDir = new File(srcStatDir,"statuslogs");
        TestUtils.copyDirectory(srcStatDir,dstStatDir,fileSystem);
      }

    
    producer = new TestDeliveryNotificationsProducer(dstStatDir,fileSystem);
  }

  @Test
  public void test() throws InterruptedException, ParseException {
    producer.addListener(this);
    notifications.clear();
    producer.start();
    synchronized (this) {
        wait(1000);
    }
    producer.shutdown();

    assertTrue(notifications.size()==5);

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


    DeliveryNotification n = notifications.get(0);
    assertTrue(n.getEventDate().equals(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083015")));
    assertTrue(n.getDeliveryId()==1);
    assertTrue(n.getUserId().equals("a"));
    assertTrue(n.getType()==DeliveryNotificationType.DELIVERY_START);

    //18,1,"a",0,1,D,0,+79130000001,userdata1
    DeliveryMessageNotification m = (DeliveryMessageNotification) notifications.get(1);
    assertTrue(m.getEventDate().equals(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083018")));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getType()== DeliveryNotificationType.MESSAGE_FINISHED);
    assertTrue(m.getMessageId()==1);
    assertTrue(m.getMessageState()==MessageState.Delivered);
    assertTrue(m.getSmppStatus()==0);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000001"));
    assertTrue(m.getUserData().equals("userdata1=1"));

    //18,1,"a",0,2,F,1,+79130000002,userdata2
    m = (DeliveryMessageNotification) notifications.get(2);
    assertTrue(m.getEventDate().equals(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083018")));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getType()==DeliveryNotificationType.MESSAGE_FINISHED);
    assertTrue(m.getMessageId()==2);
    assertTrue(m.getMessageState()==MessageState.Failed);
    assertTrue(m.getSmppStatus()==1);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000002"));
    assertTrue(m.getUserData().equals("userdata2=2"));

    //20,1,"a",0,3,E,2,+79130000003
    m = (DeliveryMessageNotification) notifications.get(3);
    assertTrue(m.getEventDate().equals(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083020")));
    assertTrue(m.getDeliveryId()==1);
    assertTrue(m.getUserId().equals("a"));
    assertTrue(m.getType()==DeliveryNotificationType.MESSAGE_FINISHED);
    assertTrue(m.getMessageId()==3);
    assertTrue(m.getMessageState()==MessageState.Expired);
    assertTrue(m.getSmppStatus()==2);
    assertTrue(m.getAddress().getSimpleAddress().equals("+79130000003"));
    assertTrue(m.getUserData()==null);

//35,2,"b",2
     n = notifications.get(4);
    assertTrue(n.getEventDate().equals(new SimpleDateFormat("yyyyMMddHHmmss").parse("20101112083035")));
    assertTrue(n.getDeliveryId()==2);
    assertTrue(n.getUserId().equals("b"));
    assertTrue(n.getType()==DeliveryNotificationType.DELIVERY_FINISHED);

  }



  @AfterClass
  public static void shutdown() {
    if(dstStatDir != null) {
      TestUtils.recursiveDeleteFolder(dstStatDir);
    }
  }

  public void onDeliveryStartNotification(DeliveryNotification notification) {
    notifications.add(notification);
  }

  public void onDeliveryFinishNotification(DeliveryNotification notification) {
    notifications.add(notification);
  }

  public void onMessageNotification(DeliveryMessageNotification notification) {
    notifications.add(notification);
  }

  
}
