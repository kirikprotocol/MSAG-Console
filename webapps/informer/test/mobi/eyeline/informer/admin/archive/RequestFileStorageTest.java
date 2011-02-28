package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfigException;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.text.ParseException;
import java.util.Date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class RequestFileStorageTest {

  private RequestFileStorage storage;

  private File testDir;

  @Before
  public void before() throws InitException {
    testDir = TestUtils.createRandomDir("req_file_st_test");
    storage = new RequestFileStorage(testDir, new TestFileSystem());
  }

  @After
  public void after() {
    if(testDir != null) {
      TestUtils.recursiveDeleteFolder(testDir);
    }
  }

  private static DeliveriesRequest createDeliveriesRequest() {
    DeliveriesRequest r = new DeliveriesRequest();
    r.setCreater("me");
    r.setDeliveryId(1024);
    r.setDeliveryName("my_delivery");
    r.setFrom(new Date(12313131));
    r.setTill(new Date(43434343));
    r.setId(1);
    r.setName("my_request");
    r.setOwner("a");
    r.setStatus(DeliveriesRequest.Status.IN_PROCESS);
    return r;
  }

  private static MessagesRequest createMessagesRequest() {
    MessagesRequest r = new MessagesRequest();
    r.setCreater("me");
    r.setAddress(new Address("+79139489906"));
    r.setFrom(new Date(12313131));
    r.setTill(new Date(43434343));
    r.setId(1);
    r.setName("my_request");
    r.setStatus(DeliveriesRequest.Status.IN_PROCESS);
    return r;
  }



  @Test
  public void saveDeliveriesRCheck() throws AdminException {
    DeliveriesRequest r = createDeliveriesRequest();

    storage.createRequest(r);

    Request r1 = storage.getRequest(r.getId());
    assertEquals(r, r1);

    assertTrue("File is not exist", new File(testDir, RequestFileStorage.buildFileName(r)).exists());


  }


  @Test
  public void saveMessagesRCheck() throws AdminException {
    MessagesRequest r = createMessagesRequest();

    storage.createRequest(r);

    Request r1 = storage.getRequest(r.getId());
    assertEquals(r, r1);

    assertTrue("File is not exist", new File(testDir, RequestFileStorage.buildFileName(r)).exists());


  }

  @Test
  public void saveDeliveriesRFileCheck() throws AdminException, ParseException, XmlConfigException {
    DeliveriesRequest r = createDeliveriesRequest();

    storage.createRequest(r);

    File f = new File(testDir, RequestFileStorage.buildFileName(r));
    assertTrue("File is not exist", f.exists());

    Request r1 = storage.loadRequest(f);

    assertEquals(r, r1);

  }
  @Test
  public void saveMessagesRFileCheck() throws AdminException, ParseException, XmlConfigException {
    MessagesRequest r = createMessagesRequest();

    storage.createRequest(r);

    File f = new File(testDir, RequestFileStorage.buildFileName(r));
    assertTrue("File is not exist", f.exists());

    Request r1 = storage.loadRequest(f);

    assertEquals(r, r1);

  }

  @Test
  public void deleteDeliveriesRFileCheck() throws AdminException, ParseException, XmlConfigException {
    DeliveriesRequest r = createDeliveriesRequest();

    storage.createRequest(r);

    storage.removeRequest(r.getId());

    File f = new File(testDir, RequestFileStorage.buildFileName(r));
    assertTrue("File is exist", !f.exists());

  }
  @Test
  public void deleteMessagesRFileCheck() throws AdminException, ParseException, XmlConfigException {
    MessagesRequest r = createMessagesRequest();

    storage.createRequest(r);

    storage.removeRequest(r.getId());

    File f = new File(testDir, RequestFileStorage.buildFileName(r));
    assertTrue("File is exist", !f.exists());
  }
}
