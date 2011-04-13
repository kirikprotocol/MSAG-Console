package ru.sibinco.smsx.stats.backend;

import junit.framework.TestCase;
import ru.sibinco.smsx.stats.backend.datasource.*;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class StatRequestManagerTest extends TestCase{
  
private StatRequestManager requestsManager;
  
  private File testDir;

  
  public void setUp() throws Exception {
    testDir = TestUtils.createTestDir("stats");
    requestsManager = new StatRequestManager(testDir, testDir);
  }


  public void tearDown() {
    requestsManager.shutdown();
    if(testDir != null) {
      TestUtils.recursiveDirRemove(testDir);
    }
  }



  private static SmsxRequestPrototype createSmxPrototype() {
    SmsxRequestPrototype q = new SmsxRequestPrototype();
    q.setFrom(new Date(0));
    q.setTill(new Date(System.currentTimeMillis()+121213313l));
    return q;
  }

  public void testAddSmsxCheck() throws Exception {

    SmsxRequestPrototype q = createSmxPrototype();

    SmsxRequest request = requestsManager.createRequest(q);

    assertNotNull(requestsManager.getRequest(request.getId()));

  }

  
  public void testRemoveSmsxCheck() throws Exception {

    SmsxRequestPrototype q = createSmxPrototype();

    SmsxRequest request = requestsManager.createRequest(q);

    requestsManager.removeRequest(request.getId());

    assertNull(requestsManager.getRequest(request.getId()));

  }

  
  public void testCreatedSmsxStatus() throws Exception {
    SmsxRequestPrototype q = createSmxPrototype();

    SmsxRequest request = requestsManager.createRequest(q);

    assertNotNull(requestsManager.getRequest(request.getId()).getStatus());

    assertNotSame(requestsManager.getRequest(request.getId()).getStatus(), SmsxRequest.Status.CANCELED);

  }

  
  public void testCancelSmsx() throws Exception {
    SmsxRequestPrototype q = createSmxPrototype();
    SmsxRequest request = requestsManager.createRequest(q);
    requestsManager.cancelRequest(request.getId());
    assertNotSame(requestsManager.getRequest(request.getId()).getStatus(), SmsxRequest.Status.IN_PROCESS);
  }

  public void testCheckSmxResult() throws Exception {
    SmsxRequestPrototype q = createSmxPrototype();
    SmsxRequest request = requestsManager.createRequest(q);

    SmsxRequest r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = requestsManager.getRequest(request.getId());
    }while (r.getStatus() != SmsxRequest.Status.READY);

    assertEquals(r.getProgress(), 100);
  }

  public void testSmsxResult() throws Exception{
    createWebSmsArtefacts();
    createTrafficArtefacts();
    createSmsxUsersArtefacts();

    SmsxRequestPrototype q = createSmxPrototype();
    SmsxRequest request = requestsManager.createRequest(q);

    SmsxRequest r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = requestsManager.getRequest(request.getId());
    }while (r.getStatus() != SmsxRequest.Status.READY);

    SmsxResults results = requestsManager.getSmsxResult(r.getId());

    final List found = new LinkedList();
    results.getTraffic(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        found.add(o);
        return true;
      }
    });
    assertTraffic(found);
    found.clear();

    results.getSmsxUsers(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        found.add(o);
        return true;
      }
    });
    assertSmsxUsers(found);
    found.clear();

    results.getWebDaily(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        found.add(o);
        return true;
      }
    });
    assertWebDaily(found);
    found.clear();

    results.getWebRegions(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        found.add(o);
        return true;
      }
    });
    assertWebRegions(found);
  }


  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

  private void assertTraffic(Collection found) {
    Set expected = new HashSet();
    expected.add(new Traffic("МР Дальний Восток-Якутск",0,false,440));
    expected.add(new Traffic("МР Дальний Восток-Якутск",1,false,320));
    expected.add(new Traffic("МР Дальний Восток-Якутск",0,true,120));

    assertEquals(found.size(), expected.size());
    Iterator i = found.iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);
  }

  private void assertSmsxUsers(Collection found) {
    Set expected = new HashSet();
    expected.add(new SmsxUsers(0, "МР Сибирь-Новосибирск", 2));
    expected.add(new SmsxUsers(1, "МР Сибирь-Новосибирск", 1));
    expected.add(new SmsxUsers(0, "Unknown", 1));

    assertEquals(found.size(), expected.size());
    Iterator i = found.iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);

  }

  private void assertWebRegions(Collection found) {
    Set expected = new HashSet();
    expected.add(new WebRegion("МР Москва-Москва", true, 1, 2));
    expected.add(new WebRegion("МР Москва-Москва", false, 0, 1));
    expected.add(new WebRegion("Unknown", false, 1, 0));

    assertEquals(found.size(), expected.size());
    Iterator i = found.iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);
  }

  private void assertWebDaily(Collection found) {
    Set expected = new HashSet();
    expected.add(new WebDaily(sdf.format(date), "МР Дальний Восток-Якутск", false, 760));
    expected.add(new WebDaily(sdf.format(date), "МР Дальний Восток-Якутск", true, 120));

    assertEquals(found.size(), expected.size());
    Iterator i = found.iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);

  }

  private void createTrafficArtefacts() throws Exception{
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
          new File(testDir, new StringBuffer().append(sdf.format(date)).append("-traffic.csv").toString())
      ), "windows-1251")));
      writer.println("SERVICE_ID,MSC,REGION,SRC_SME_ID");
      writer.println("0,0,МР Дальний Восток-Якутск,websms,220");
      writer.println("0,0,МР Дальний Восток-Якутск,websyssms,220");
      writer.println("1,0,МР Дальний Восток-Якутск,websms,320");
      writer.println("0,1,МР Дальний Восток-Якутск,websms,120");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

  private void createSmsxUsersArtefacts() throws Exception{
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
          new File(testDir, new StringBuffer().append(sdf.format(date)).append("-smsx-users.csv").toString())
      ), "windows-1251")));
      writer.println("SERVICE_ID,SRC_ADDRESS,REGION");
      writer.println("0,.1.1.79139095312,МР Сибирь-Новосибирск");
      writer.println("1,.1.1.79139095683,МР Сибирь-Новосибирск");
      writer.println("0,.1.1.79139489906,МР Сибирь-Новосибирск");
      writer.println("0,.1.1.79109424802,Unknown");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

  private void createWebSmsArtefacts() throws Exception{
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
          new File(testDir, new StringBuffer().append(sdf.format(date)).append("-websms-users.csv").toString())
      ),"windows-1251")));
      writer.println("INDEX,MSC,ADDRESS,REGION");
      writer.println("1,1,.1.1.79169112907,МР Москва-Москва");
      writer.println("2,1,.1.1.79169112908,МР Москва-Москва");
      writer.println("2,1,.1.1.79169112901,МР Москва-Москва");
      writer.println("2,0,.1.1.79169112901,МР Москва-Москва");
      writer.println("1,0,.1.1.79169112901,Unknown");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }
  
}
