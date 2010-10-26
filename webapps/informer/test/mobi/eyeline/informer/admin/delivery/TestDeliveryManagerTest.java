package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.Date;
import java.util.LinkedList;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManagerTest {


  private static TestDeliveryManager manager;

  @BeforeClass
  public static void init() {
    manager = new TestDeliveryManager();
  }

  @AfterClass
  public static void shutdown() {
    if(manager != null) {
      manager.shutdown();
    }
  }

  @Test
  public void delivery() throws AdminException, InterruptedException {

    Delivery d = _createDelivery();

    checkStats(d, new DeliveryStatus[]{DeliveryStatus.Active, DeliveryStatus.Finished});

    checkGetDeliviries(d, new DeliveryStatus[]{DeliveryStatus.Active, DeliveryStatus.Finished});

    checkGetMessages(d, new MessageState[]{MessageState.New, MessageState.Delivered, MessageState.Failed});

    Thread.sleep(60000);

    checkStats(d, new DeliveryStatus[]{DeliveryStatus.Finished});

    checkGetMessages(d, new MessageState[]{MessageState.Delivered, MessageState.Failed});

    checkGetDeliviries(d, new DeliveryStatus[]{DeliveryStatus.Finished});

    manager.dropDelivery("","", d.getId());

    assertNull(manager.getDelivery("","",d.getId()));
    
  }

  private void checkGetMessages(final Delivery d, MessageState[] states) throws AdminException{

    MessageFilter messageFilter = new MessageFilter();
    messageFilter.setStartDate(d.getStartDate());
    messageFilter.setEndDate(d.getEndDate());
    messageFilter.setFields(new MessageFields[]{MessageFields.State});
    messageFilter.setDeliveryId(d.getId());
    messageFilter.setStates(states);

    DeliveryDataSource<MessageInfo> ms = manager.getMessagesStates("","", messageFilter, 10);
    {
      final boolean[] nonEmpty = new boolean[]{false};
      ms.visit(new DeliveryDataSource.Visitor<MessageInfo>() {
        public boolean visit(MessageInfo value) throws AdminException {
          nonEmpty[0] = true;
          assertTrue(value.getAbonent().equals("+79139489906") || value.getAbonent().equals("+79139489907"));
          return true;
        }
      });
      assertTrue(nonEmpty[0]);
    }
    assertEquals(manager.countMessages("","", messageFilter), 2);
  }

  private void checkGetDeliviries(final Delivery d1, DeliveryStatus[] statuses) throws AdminException{

    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(new String[]{"Test delivery", "Dsadsaasd"});
    filter.setStatusFilter(statuses);
    filter.setUserIdFilter(new String[]{"me"});
    filter.setResultFields(new DeliveryFields[]{DeliveryFields.Name});

    DeliveryDataSource<DeliveryInfo> ds = manager.getDeliveries("","",filter, 10);
    {
      final boolean[] nonEmpty = new boolean[]{false};
      ds.visit(new DeliveryDataSource.Visitor<mobi.eyeline.informer.admin.delivery.DeliveryInfo>() {
        public boolean visit(DeliveryInfo value) throws AdminException {
          nonEmpty[0] = true;
          assertEquals(value.getDeliveryId(), d1.getId().intValue());
          return true;
        }
      });
      assertTrue(nonEmpty[0]);
    }
    assertTrue(manager.countDeliveries("","", filter) >= 1);
  }

  private void checkStats(Delivery d, DeliveryStatus[] statuses) throws AdminException{
    DeliveryStatistics st = manager.getDeliveryStats("","", d.getId());
    System.out.println("Delivered: "+st.getDeliveredMessages());
    System.out.println("Expired: "+st.getExpiredMessages());
    System.out.println("New: "+st.getNewMessages());
    System.out.println("Failed: "+st.getFailedMessage());
    System.out.println("Processed: "+st.getProcessMessage());
    assertEquals(st.getDeliveredMessages()+st.getExpiredMessages()+st.getNewMessages()+st.getFailedMessage()+st.getProcessMessage(), 2);
    DeliveryStatus current = st.getDeliveryState().getStatus();
    boolean success = false;
    for(DeliveryStatus s : statuses) {
      if(s == current) {
        success = true;
        break;
      }
    }
    assertTrue(success);

  }

  private Delivery _createDelivery() throws AdminException{
    Delivery d = new Delivery();
    d.setActivePeriodEnd(new Date());
    d.setActivePeriodStart(new Date(0));
    d.setActiveWeekDays(new Delivery.Day[]{Delivery.Day.Fri, Delivery.Day.Sat});
    d.setDeliveryMode(DeliveryMode.SMS);
    d.setEndDate(new Date(System.currentTimeMillis() + 1000000));
    d.setName("Test delivery");
    d.setOwner("me");
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setRetryPolicy("policy1");
    d.setStartDate(new Date());
    d.setSvcType("svc1");
    d.setValidityDate(new Date());
    d.setValidityPeriod("1:00:00");

    manager.createDelivery("","", d, new MessageDataSource() {
      private LinkedList<Message> ms = new LinkedList<Message>() {
        {
          Message m1 = Message.newTextMessage("text1");
          m1.setAbonent(new Address("+79139489906"));
          add(m1);
          Message m2 = Message.newTextMessage("text2");
          m2.setAbonent(new Address("+79139489907"));
          add(m2);
        }
      };

      public Message next() throws AdminException {
        if(ms.isEmpty()) {
          return null;
        }
        return ms.removeFirst();
      }
    }, null);
    
    assertNotNull(d.getId());
    manager.activateDelivery("","",d.getId());


    final Delivery d1 = manager.getDelivery("","",d.getId());
    assertEquals(d1, d);

    return d;
  }

}
