package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.*;

import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;


import java.io.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 18.11.2010
 * Time: 14:18:35
 */
public class ContentProviderTest {
  static AdminContext context;
  static File appDir;
  static File userSrcDir;
  static File userLocalDir;
  static UserCPsettings ucps;

  @BeforeClass
  public static void init() throws InitException, AdminException {
    appDir = TestUtils.createRandomDir(".context.backup");
    context = new TestAdminContext(appDir);
    userSrcDir = new File(appDir,"userSrcDir");
    context.getFileSystem().mkdirs(userSrcDir);
    User u= context.getUser("a");
    u.setImportDeliveriesFromDir(true);
    u.setAllRegionsAllowed(false);
    u.setCreateReports(true);
    List<Integer> regions = new ArrayList<Integer>();
    regions.add(0);
    u.setRegions(regions);
    Region r = context.getRegion(0);
    r.addMask(new Address("+7913???????"));
    context.updateRegion(r);

    List<UserCPsettings> ucpSettings = new ArrayList<UserCPsettings>();
    ucps = new UserCPsettings();
    ucps.setProtocol(UserCPsettings.Protocol.file);
    ucps.setDirectory(userSrcDir.getName());
    ucps.setHost("localhost");
    ucps.setPort(0);
    ucps.setLogin("");
    ucps.setPassword("");
    ucps.setEncoding("UTF-8");
    ucps.setSourceAddress(new Address("+79130000000"));
    ucpSettings.add(ucps);
    u.setCpSettings(ucpSettings);

    List<Integer> days= new ArrayList<Integer>();
    for(int i=0;i<7;i++) {
      days.add(i);      
    }
    u.setDeliveryDays(days);

    u.setDeliveryStartTime(new Time(0,0,0));
    u.setDeliveryEndTime(new Time(24,0,0));
    context.updateUser(u);


  }

//  @Test
//  public void testStartStop() throws AdminException {
//    ContentProviderDaemon cp = getCPDaemon();
//    assertTrue(cp!=null);
//    assertTrue(cp.isStarted()==false);
//    cp.start();
//    assertTrue(cp.isStarted()==true);
//    cp.stop();
//    assertTrue(cp.isStarted()==false);
//  }



  @Test
  public void testImport() throws AdminException, UnsupportedEncodingException, InterruptedException {
    ContentProviderDaemon cpDaemon = getCPDaemon();
    userLocalDir = cpDaemon.getUserLocalDir("a",ucps);

    createTestInputFile();


    cpDaemon.start();
    Date startDate = new Date();

    synchronized (this) {
      try {
        wait(2000);
      }
      catch (InterruptedException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      }
    }

    String[] files = context.getFileSystem().list(userSrcDir);
    String fn=null;
    for(String fName : files) {
      if(fName.indexOf(".bak")>=0) {
        fn = fName;
      }
    }
    assertEquals("test.csv.bak",fn);

    files = context.getFileSystem().list(userLocalDir);
    fn=null;
    for(String fName : files) {
      if(fName.indexOf(".rep.")>=0) {
        fn = fName;
      }
    }
    assertTrue(fn!=null);

    int id = Integer.valueOf(fn.substring(fn.indexOf(".rep.")+5));
    Delivery d = context.getDelivery("a","1",id);
    assertTrue(d!=null);

    MessageFilter filter = new MessageFilter(id,startDate,new Date());
    final Counter cnt = new Counter();
    context.getMessagesStates("a","1",filter,1000,new Visitor<Message>(){
      public boolean visit(Message mi) throws AdminException {
        assertTrue(mi.getAbonent().getSimpleAddress().startsWith("+7913000"));
        String s = mi.getAbonent().getSimpleAddress().substring(5);
        int n = Integer.parseInt(s,10);
        assertEquals(mi.getText(),"Привет с большого бодуна "+n);
        cnt.inc();
        return true;
      }
    });
    assertEquals(cnt.getN(),2000);

    cpDaemon.stop();


    //pahse 2 test report creation


    ChangeDeliveryStatusEvent stateEventChange = new ChangeDeliveryStatusEvent(DeliveryStatus.Finished,new Date(),id,"a");
    cpDaemon.deliveryStateChanged(stateEventChange);

    File workDir = new File(context.getWorkDir(),"contentProvider");
    assertTrue(context.getFileSystem().exists(workDir));

    File notificationFile = new File(workDir,id+".notification");
    assertTrue(context.getFileSystem().exists(notificationFile));

    cpDaemon.start();

    synchronized (this) {wait(5000);}

    assertTrue(context.getFileSystem().exists(new File(userSrcDir,"test.report"))) ;

    cpDaemon.stop();
  }


  private ContentProviderDaemon getCPDaemon() {
    List<Daemon> daemons = context.getDaemons();
    ContentProviderDaemon cp = null;
    for(Daemon d : daemons) {
      if(d.getName().equals(ContentProviderDaemon.NAME)) {
        cp = (ContentProviderDaemon) d;
      }
    }
    return cp;
  }

  @AfterClass
  public static void shutdown() {
    context.shutdown();
    if(appDir!=null) TestUtils.recursiveDeleteFolder(appDir);
  }

  public class Counter {
    int n = 0;
    public void inc() {
      n++;
    }

    public int getN() {
      return n;
    }
  }

  private void createTestInputFile() throws AdminException, UnsupportedEncodingException {
    User u= context.getUser("a");

    File userFile = new File(userSrcDir,"test.tmp");
    PrintStream ps = new PrintStream(context.getFileSystem().getOutputStream(userFile,false),false,"UTF-8");
    for(int i=0; i<2000 ; i++ ) {
      String sn = ""+i;
      while(sn.length()<4) sn='0'+sn;
      ps.println("+7913000"+sn+"| Привет с большого бодуна "+i);//
    }
    ps.flush();
    ps.close();
    context.getFileSystem().rename(userFile,new File(userSrcDir,"test.csv"));
  }
  
}