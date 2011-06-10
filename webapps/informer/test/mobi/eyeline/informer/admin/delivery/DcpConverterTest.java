package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryListInfo;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessage;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.util.Arrays;
import java.util.Date;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class DcpConverterTest {

  @Test
  public void testTime() throws AdminException {
    String t = "10:00:09";
    assertEquals(DcpConverter.convertTimeToDcpFormat(DcpConverter.convertTimeFromDcpFormat(t)), t);
  }

  @Test
  public void testDate() throws AdminException {
    String date = "12.11.2007 10:00:09";
    assertEquals(DcpConverter.convertDateToDcpFormat(DcpConverter.convertDateFromDcpFormat(date)), date);
  }

  @Test
  public void testConvertDay() throws AdminException {
    Day[] ds1 = new Day[]{Day.Sun, Day.Tue};
    assertTrue(Arrays.equals(ds1, DcpConverter.convertDays(DcpConverter.convertDays(ds1))));
  }

  @Test
  public void testConvertDeliveryMode() throws AdminException {
    for(DeliveryMode m : DeliveryMode.values()) {
      assertEquals(m, DcpConverter.convert(DcpConverter.convert(m)));
    }
  }

  private static Delivery createDelivery() {
    Delivery d = new Delivery();
//    d.setType(Delivery.Type.IndividualTexts);
    d.setSourceAddress(new Address("+791394"));
    d.setActivePeriodEnd(new Time(22,0,0));
    d.setActivePeriodStart(new Time(0,0,0));
    d.setActiveWeekDays(new Day[]{Day.Fri, Day.Sat});
    d.setDeliveryMode(DeliveryMode.SMS);
    d.setEndDate(new Date(System.currentTimeMillis() + 1000000));
    d.setId(0);
    d.setName("Test delivery");
    d.setOwner("me");
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setRetryPolicy("policy1");
    d.setStartDate(new Date());
    d.setSvcType("svc1");
    d.setValidityPeriod(new Time(1,0,0));
    d.setMessageTimeToLive(new Time(2,0,0));
    d.setCreateDate(new Date(121212));
    d.setProperty(UserDataConsts.RESTRICTION, "true");
    d.setBoundToLocalTime(true);
    return d;
  }

  @Test
  public void testConvertDelivery() throws AdminException {
    Delivery d = createDelivery();
    assertEquals(d, DcpConverter.convert(d.getId(), DcpConverter.convert(d)));
  }
  @Test
  public void testConvertDeliveryWNullCreateDate() throws AdminException {
    Delivery d = createDelivery();
    d.setCreateDate(null);
    Delivery d2 = DcpConverter.convert(d.getId(), DcpConverter.convert(d));
    assertEquals(d2.getCreateDate().getTime()/1000, d.getStartDate().getTime()/1000);
    d.setCreateDate(d2.getCreateDate());
    assertEquals(d, d2);
  }


  @Test
  public void testDeliveryState() throws AdminException {
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.Paused);
    state.setDate(new Date());
    assertEquals(state, DcpConverter.convert(DcpConverter.convert(state)));
  }

  @Test
  public void testConvertMessageInfo() throws AdminException {
    Date date = new Date();
    mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi = new mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo();
    mi.setAbonent("+79139489906");
    mi.setDate(DcpConverter.convertDateToDcpFormat(date));
    mi.setErrorCode(1179);
    mi.setId(1);
    mi.setText("sms_text");
    mi.setState(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessageState.Failed);
    mi.setUserData("user=data");
    Message info = DcpConverter.convert(mi);
    assertEquals(mi.getAbonent(), info.getAbonent().getSimpleAddress());
    assertEquals(DcpConverter.convertDateFromDcpFormat(mi.getDate()), info.getDate());
    assertNotNull(info.getErrorCode());
    assertEquals(mi.getErrorCode(), info.getErrorCode().intValue());
    assertEquals(mi.getId(), info.getId().longValue());
    assertEquals(mi.getText(), info.getText());
    assertEquals(DcpConverter.convert(mi.getState()), info.getState());
  }


  @Test
  public void convertMessageState() throws AdminException{
    for(MessageState state : MessageState.values()) {
      assertEquals(state, DcpConverter.convert(DcpConverter.convert(state)));
    }
  }

  @Test
  public void convertMessage() throws AdminException {
    Message m = new Message();
    m.setId(34L);
    m.setAbonent(new Address("2313123"));
    m.setText("sdaasdass sdadas sadgdxvxc");
    DeliveryMessage dm = DcpConverter.convert(m);
    assertEquals(dm.getAbonent(), m.getAbonent().getSimpleAddress());
    assertEquals(dm.getText(), m.getText());
  }


  @Test
  public void testConvertDeliveryInfo() throws AdminException {
    DeliveryListInfo di = new DeliveryListInfo();
    di.setActivityPeriodEnd(DcpConverter.convertTimeToDcpFormat(new Time(new Date())));
    di.setActivityPeriodStart(DcpConverter.convertTimeToDcpFormat(new Time(new Date(1000003231L))));
    di.setDeliveryId(45);
    di.setEndDate(DcpConverter.convertDateToDcpFormat(new Date(System.currentTimeMillis() + 1000000)));
    di.setName("test delivery");
    di.setStartDate(DcpConverter.convertDateToDcpFormat(new Date()));
    di.setStatus(DcpConverter.convert(DeliveryStatus.Cancelled));
    di.setUserId("user2");
    Delivery info = DcpConverter.convert(di);
    assertEquals(DcpConverter.convertTimeToDcpFormat(info.getActivePeriodEnd()), di.getActivityPeriodEnd());
    assertEquals(DcpConverter.convertTimeToDcpFormat(info.getActivePeriodStart()), di.getActivityPeriodStart());
    assertEquals(DcpConverter.convertDateToDcpFormat(info.getStartDate()), di.getStartDate());
    assertEquals(DcpConverter.convertDateToDcpFormat(info.getEndDate()), di.getEndDate());
    assertEquals(DcpConverter.convert(info.getStatus()), di.getStatus());
    assertEquals(info.getId().intValue(), di.getDeliveryId());
    assertEquals(info.getName(), di.getName());
    assertEquals(info.getOwner(), di.getUserId());
  }

  @Test
  public void testConvertDeliveryStatus() {
    for(DeliveryStatus ds : DeliveryStatus.values()) {
      assertEquals(ds, DcpConverter.convert(DcpConverter.convert(ds)));
    }
  }


  @Test
  public void testConvertDeliveryStat() throws AdminException {
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics stats = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics();
    stats.setDeliveredMessages(12);
    stats.setExpiredMessages(13);
    stats.setFailedMessage(14);
    stats.setNewMessages(15);
    stats.setProcessMessage(16);
    stats.setSentMessages(2112);
    stats.setRetriedMessages(3223);
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState ds = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState();
    ds.setDate(DcpConverter.convertDateToDcpFormat(new Date()));
    ds.setStatus(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus.Active);

    DeliveryStatistics s = DcpConverter.convert(stats, ds);
    assertEquals(s.getFailedMessages(), stats.getFailedMessage());
    assertEquals(s.getProcessMessages(), stats.getProcessMessage());
    assertEquals(s.getDeliveredMessages(), stats.getDeliveredMessages());
    assertEquals(s.getExpiredMessages(), stats.getExpiredMessages());
    assertEquals(s.getNewMessages(), stats.getNewMessages());
    assertEquals(s.getSentMessages(), stats.getSentMessages());
    assertEquals(s.getRetriedMessages(), stats.getRetriedMessages());

    assertEquals(s.getDeliveryState(), DcpConverter.convert(ds));
  }

}
