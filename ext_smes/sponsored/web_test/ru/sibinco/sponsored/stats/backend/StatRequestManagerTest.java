package ru.sibinco.sponsored.stats.backend;

import junit.framework.TestCase;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;

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


  private static SponsoredRequestPrototype createSponsoredPrototype() {
    SponsoredRequestPrototype q = new SponsoredRequestPrototype();
    q.setFrom(new Date(0));
    q.setTill(new Date(System.currentTimeMillis() + 121213313l));
    q.setCost(1.6f);
    return q;
  }

  
  public void testAddSponsoredCheck() throws Exception {

    SponsoredRequestPrototype q = createSponsoredPrototype();

    SponsoredRequest request = requestsManager.createRequest(q);

    assertNotNull(requestsManager.getRequest(request.getId()));

  }
  
  public void testRemoveSponsoredCheck() throws Exception {

    SponsoredRequestPrototype q = createSponsoredPrototype();

    SponsoredRequest request = requestsManager.createRequest(q);

    requestsManager.removeRequest(request.getId());

    assertNull(requestsManager.getRequest(request.getId()));

  }

  
  public void testCreatedSponsoredStatus() throws Exception {
    SponsoredRequestPrototype q = createSponsoredPrototype();

    SponsoredRequest request = requestsManager.createRequest(q);

    assertNotNull(requestsManager.getRequest(request.getId()).getStatus());

    assertNotSame(requestsManager.getRequest(request.getId()).getStatus(), SponsoredRequest.Status.CANCELED);

  }

  public void testCancelSponsored() throws Exception {
    SponsoredRequestPrototype q = createSponsoredPrototype();
    SponsoredRequest request = requestsManager.createRequest(q);
    requestsManager.cancelRequest(request.getId());
    assertNotSame(requestsManager.getRequest(request.getId()).getStatus(), SponsoredRequest.Status.IN_PROCESS);
  }

  public void testCheckSponsoredResult() throws Exception {
    SponsoredRequestPrototype q = createSponsoredPrototype();
    SponsoredRequest request = requestsManager.createRequest(q);

    SponsoredRequest r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = requestsManager.getRequest(request.getId());
    }while (r.getStatus() != SponsoredRequest.Status.READY);

    assertEquals(r.getProgress(), 100);
  }


  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");


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
