package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.*;

import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
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


  @BeforeClass
  public static void init() throws InitException, AdminException {
    appDir = TestUtils.createRandomDir(".context.backup");
    context = new TestAdminContext(appDir);
    context.getFileSystem().mkdirs(new File(appDir,"userDir"));
    User u= context.getUser("a");
    u.setFileEncoding("cp1251");
    u.setImportDeliveriesFromDir(true);
    u.setDirectory("userDir");
    List<Integer> days= new ArrayList<Integer>();
    days.add(1);
    days.add(2);
    days.add(3);
    u.setDeliveryDays(days);
    context.updateUser(u);

    u.setDeliveryStartTime(new Time(0,0,0));
    u.setDeliveryEndTime(new Time(22,0,0));
  }

  @Test
  public void testStartStop() throws AdminException {
    ContentProviderDaemon cp = getCPDaemon();
    assertTrue(cp!=null);
    assertTrue(cp.isStarted()==false);
    cp.start();
    assertTrue(cp.isStarted()==true);
    cp.stop();
    assertTrue(cp.isStarted()==false);   
  }

  @Test
  public void testImport() throws AdminException, UnsupportedEncodingException {
    ContentProviderDaemon cpDaemon = getCPDaemon();
    cpDaemon.start();
    Date startDate = new Date();

    User u= context.getUser("a");
    File userDir = new File(appDir,u.getDirectory());
    File userFile = new File(userDir,"test.tmp");
    PrintStream ps = new PrintStream(context.getFileSystem().getOutputStream(userFile,false),false,u.getFileEncoding());
    for(int i=0; i<2000 ; i++ ) {
      String sn = ""+i;
      while(sn.length()<4) sn='0'+sn;
      ps.println("+7913000"+sn+"| Привет с большого бодуна "+i);
    }
    ps.flush();
    ps.close();
    context.getFileSystem().rename(userFile,new File(userDir,"test.csv"));
    synchronized (this) {
      try {
        wait(2000);
      }
      catch (InterruptedException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      }
    }

    String[] files = context.getFileSystem().list(userDir);
    String fn=null;
    for(String fName : files) {
      if(fName.indexOf(".bak.")>=0) {
        fn = fName;
      }
    }
    assert(fn!=null);

    int id = Integer.valueOf(fn.substring(fn.indexOf(".bak.")+5));
    Delivery d = context.getDelivery("a","1",id);
    assertTrue(d!=null);

    MessageFilter filter = new MessageFilter(id,startDate,new Date());
    filter.setFields(new MessageFields[]{MessageFields.Abonent,MessageFields.Text});
    final Counter cnt = new Counter();
    context.getMessagesStates("a","1",filter,1000,new Visitor<MessageInfo>(){
      public boolean visit(MessageInfo mi) throws AdminException {
        assertTrue(mi.getAbonent().startsWith("+7913000"));
        String s = mi.getAbonent().substring(5);
        int n = Integer.parseInt(s,10);
        assertEquals(mi.getText(),"Привет с большого бодуна "+n);
        cnt.inc();
        return true;
      }
    });
    assertEquals(cnt.getN(),2000);
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
}
