package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
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
    manager = new TestDeliveryManager(null);
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

    checkStats(d, new DeliveryStatus[]{DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Finished});

    checkGetDeliviries(d, new DeliveryStatus[]{DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Finished});

    checkGetMessages(d, new MessageState[]{MessageState.New, MessageState.Delivered, MessageState.Failed});

    manager.forceModifyDeliveries();

    checkStats(d, new DeliveryStatus[]{DeliveryStatus.Finished});

    checkGetMessages(d, new MessageState[]{MessageState.Delivered, MessageState.Failed});

    checkGetDeliviries(d, new DeliveryStatus[]{DeliveryStatus.Finished});

    manager.dropDelivery("","", d.getId());

    assertNull(manager.getDelivery("","",d.getId()));

  }

//  @Test
//  public void setRestriction() throws AdminException, InterruptedException {
//    Delivery d = _createDelivery();
//    assertTrue(!Boolean.valueOf(d.getProperty(UserDataConsts.RESTRICTION)));
//    manager.setDeliveryRestriction("","", d.getId(), true);
//    d = manager.getDelivery("","",d.getId());
//    assertTrue(Boolean.valueOf(d.getProperty(UserDataConsts.RESTRICTION)));
//    final boolean[] ok = new boolean[]{false};
//    final int id = d.getId();
//    manager.getDeliveries("","", new DeliveryFilter(), 10000, new Visitor<DeliveryInfo>() {
//      public boolean visit(DeliveryInfo value) throws AdminException {
//        if(id == value.getDeliveryId()) {
//          ok[0] = Boolean.valueOf(value.getProperty(UserDataConsts.RESTRICTION));
//          return false;
//        }
//        return true;
//      }
//    });
//    assertTrue(ok[0]);
//    ok[0] = false;
//    manager.setDeliveryRestriction("","", d.getId(), false);
//    d = manager.getDelivery("","",d.getId());
//    assertTrue(!Boolean.valueOf(d.getProperty(UserDataConsts.RESTRICTION)));
//    manager.getDeliveries("","", new DeliveryFilter(), 10000, new Visitor<DeliveryInfo>() {
//      public boolean visit(DeliveryInfo value) throws AdminException {
//        if(id == value.getDeliveryId()) {
//          ok[0] = !Boolean.valueOf(value.getProperty(UserDataConsts.RESTRICTION));
//          return false;
//        }
//        return true;
//      }
//    });
//    assertTrue(ok[0]);
//    manager.dropDelivery("","",d.getId());
//
//  }

  private void checkGetMessages(final Delivery d, MessageState[] states) throws AdminException{

    MessageFilter messageFilter = new MessageFilter(d.getId(), d.getStartDate(),d.getEndDate());
    messageFilter.setStates(states);
    {
      final boolean[] nonEmpty = new boolean[]{false};
      manager.getMessages("","", messageFilter, 10, new Visitor<Message>() {
        public boolean visit(Message value) throws AdminException {
          nonEmpty[0] = true;
          assertTrue(value.getAbonent().getSimpleAddress().equals("+79139489906") || value.getAbonent().getSimpleAddress().equals("+79139489907"));
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

    {
      final boolean[] nonEmpty = new boolean[]{false};
      manager.getDeliveries("","",filter, 10, new Visitor<Delivery>() {
        public boolean visit(Delivery value) throws AdminException {
          nonEmpty[0] = true;
          assertEquals(value.getId(), d1.getId());
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
    System.out.println("Failed: "+st.getFailedMessages());
    System.out.println("Processed: "+st.getProcessMessages());
    assertEquals(st.getDeliveredMessages()+st.getExpiredMessages()+st.getNewMessages()+st.getFailedMessages()+st.getProcessMessages(), 2);
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
    DeliveryPrototype d = new DeliveryPrototype();
    d.setActivePeriodStart(new Time(1,0,0));
    d.setActivePeriodEnd(new Time(22,0,0));
    d.setActiveWeekDays(Day.values());
    d.setDeliveryMode(DeliveryMode.SMS);
    d.setEndDate(new Date(System.currentTimeMillis() + 300000));
    d.setStartDate(new Date(System.currentTimeMillis() - 300000));
    d.setName("Test delivery");
    d.setOwner("me");
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setRetryPolicy("policy1");
    d.setSvcType("svc1");
    d.setValidityPeriod(new Time(1,0,0));
    d.setSourceAddress(new Address("+79123942341"));

    Delivery delivery = manager.createDeliveryWithIndividualTexts("","", d, new DataSource<Message>() {
      private LinkedList<Message> ms = new LinkedList<Message>() {
        {
          Message m1 = Message.newMessage("text1");
          m1.setAbonent(new Address("+79139489906"));
          add(m1);
          Message m2 = Message.newMessage("text2");
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
    });

    assertNotNull(delivery.getId());
    manager.activateDelivery("","",delivery.getId());


    final Delivery d1 = manager.getDelivery("","",delivery.getId());
    assertEquals(d.getActivePeriodStart(), d1.getActivePeriodStart());
    assertEquals(d.getActivePeriodEnd(), d1.getActivePeriodEnd());
    assertArrayEquals(d.getActiveWeekDays(), d1.getActiveWeekDays());
    assertEquals(d.getDeliveryMode(), d1.getDeliveryMode());
    assertEquals(d.getEndDate(), d1.getEndDate());
    assertEquals(d.getStartDate(), d1.getStartDate());
    assertEquals(d.getName(), d1.getName());
    assertEquals(d.getOwner(), d1.getOwner());
    assertEquals(d.getPriority(), d1.getPriority());
    assertEquals(d.isReplaceMessage(), d1.isReplaceMessage());
    assertEquals(d.isRetryOnFail(), d1.isRetryOnFail());
    assertEquals(d.getRetryPolicy(), d1.getRetryPolicy());
    assertEquals(d.getSvcType(), d1.getSvcType());
    assertEquals(d.getValidityPeriod(), d1.getValidityPeriod());
    assertEquals(d.getSourceAddress(), d1.getSourceAddress());

    return delivery;
  }

}
