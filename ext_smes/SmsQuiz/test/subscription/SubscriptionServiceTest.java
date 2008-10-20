package subscription;

import org.junit.BeforeClass;
import org.junit.AfterClass;
import org.junit.Test;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.subscription.service.SubscriptionOnSMPPService;
import mobi.eyeline.smsquiz.subscription.service.SubscriptionOffSMPPService;
import com.eyeline.sme.handler.config.ServicesConfig;
import com.eyeline.sme.handler.config.ServicesConfigReader;
import com.eyeline.sme.handler.config.ConfigException;
import com.eyeline.sme.handler.HandlerException;
import com.eyeline.sme.handler.RequestToServiceMap;
import com.eyeline.sme.handler.SMPPService;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.IncomingObject;


import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.Message;

/**
 * author: alkhal
 */
public class SubscriptionServiceTest {


  private static ServicesConfig config;
  private static RequestToServiceMap requestToServiceMap;

  @BeforeClass
  public static void init() {
    try {
      ConnectionPoolFactory.init("conf/config.xml");
    } catch (StorageException e) {
      e.printStackTrace();
      assertTrue(false);
    }
    try {
      config = ServicesConfigReader.readConfig("conf/services.xml");
    } catch (ConfigException e) {
      e.printStackTrace();
      assertTrue(false);
    }
    try {
      requestToServiceMap = new RequestToServiceMap(config, new OutgoingQueue(20));
    } catch (HandlerException e) {
      e.printStackTrace();
      assertTrue(false);
    }
  }


  @Test
  public void testRequestMappingHelp() {
    IncomingObject incObj = createIncObj("sdadsadasasd");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    assertNotNull(e);
  }

  @Test
  public void testRequestMappingOn() {
    IncomingObject incObj = createIncObj("on");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    assertTrue(e.getService().getClass().getName().equals(SubscriptionOnSMPPService.class.getName()));
  }

  @Test
  public void testRequestMappingOff() {
    IncomingObject incObj = createIncObj("off");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    assertTrue(e.getService().getClass().getName().equals(SubscriptionOffSMPPService.class.getName()));
  }


  @Test
  public void testServiceOn() {
    IncomingObject incObj = createIncObj("on");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    SMPPService service = e.getService();
    assertNotNull(service);
    assertTrue(service.serve(e.getPattern().createRequest(incObj)));
  }

  @Test
  public void testServiceOff() {
    IncomingObject incObj = createIncObj("off");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    SMPPService service = e.getService();
    assertNotNull(service);
    assertTrue(service.serve(e.getPattern().createRequest(incObj)));
  }

  @Test
  public void testServiceTryOffAgain() {

    IncomingObject incObj = createIncObj("off");
    RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
    SMPPService service = e.getService();
    assertNotNull(service);
    assertFalse(service.serve(e.getPattern().createRequest(incObj)));
  }

  @AfterClass
  public static void shutdown() {
  }

  private class TestIncomingObject implements IncomingObject {
    private Message message;

    public TestIncomingObject(Message message) {
      this.message = message;
    }

    public Message getMessage() {
      return message;
    }

    public boolean isResponded() {
      return false;
    }

    public void respond(int status) throws SMPPException {
    }

  }

  private IncomingObject createIncObj(String text) {
    Message message = new Message();
    message.setDestinationAddress("177");
    message.setSourceAddress("+7913948test");
    message.setMessageString(text);
    return new TestIncomingObject(message);
  }
}
