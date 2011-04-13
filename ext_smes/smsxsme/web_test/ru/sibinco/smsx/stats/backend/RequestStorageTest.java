package ru.sibinco.smsx.stats.backend;

import junit.framework.TestCase;

import java.io.File;
import java.util.Date;
import java.util.HashSet;

/**
 * @author Aleksandr Khalitov
 */
public class RequestStorageTest extends TestCase{
  
  private RequestStorage storage;

  private File testDir;

  
  public void setUp() throws Exception {
    testDir = TestUtils.createTestDir("req_file_st_test");
    storage = new RequestStorage(testDir);
  }

  public void tearDown() {
    if(testDir != null) {
      TestUtils.recursiveDirRemove(testDir);
    }
  }

  private static SmsxRequest createSmsxRequest() {
    SmsxRequest request = new SmsxRequest();
    request.setError("sdadsdas");
    request.setFrom(new Date(0));
    request.setTill(new Date(2141241214124l));
    request.setId(12);
    request.setProgress(12123);
    request.setStatus(SmsxRequest.Status.ERROR);
    request.setReportTypesFilter(new HashSet() {{
      add(SmsxRequest.ReportType.SMSX_USERS);
      add(SmsxRequest.ReportType.WEB_DAILY);
    }});
    request.setServiceIdFilter(new HashSet() {{
      add(new Integer(1));
      add(new Integer(2));
    }});
    return request;
  }
  
  
  public void testSaveDeliveriesRCheck() throws Exception {
    SmsxRequest r = createSmsxRequest();

    storage.createRequest(r);

    SmsxRequest r1 = storage.getRequest(r.getId());
    assertEquals(r, r1);

    assertTrue("File is not exist", storage.buildFile(r).exists());


  }


  public void testSaveDeliveriesRFileCheck() throws Exception {
    SmsxRequest r = createSmsxRequest();

    storage.createRequest(r);

    File f = storage.buildFile(r);
    assertTrue("File is not exist", f.exists());

    SmsxRequest r1 = storage.loadRequest(f.getParentFile());

    assertEquals(r, r1);

  }

  
  public void testDeleteDeliveriesRFileCheck() throws Exception {
    SmsxRequest r = createSmsxRequest();

    storage.createRequest(r);

    storage.removeRequest(r.getId());

    File f = storage.buildFile(r);
    assertTrue("File is exist", !f.exists());

  }
  
  
}
