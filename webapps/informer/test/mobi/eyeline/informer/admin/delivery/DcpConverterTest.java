package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryListInfo;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessage;
import mobi.eyeline.informer.util.Address;
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
    assertEquals(DcpConverter.convertTime(DcpConverter.convertTime(t)), t);
  }

  @Test
  public void testDate() throws AdminException {
    String date = "12:11:2007 10:00:09";
    assertEquals(DcpConverter.convertDate(DcpConverter.convertDate(date)), date);
  }

  @Test
  public void testConvertDay() throws AdminException {
    Delivery.Day[] ds1 = new Delivery.Day[]{Delivery.Day.Sun, Delivery.Day.Tue};
    assertTrue(Arrays.equals(ds1, DcpConverter.convertDays(DcpConverter.convertDays(ds1))));
  }

  @Test
  public void testConvertDeliveryMode() throws AdminException {
    for(DeliveryMode m : DeliveryMode.values()) {
      assertEquals(m, DcpConverter.convert(DcpConverter.convert(m)));
    }
  }

  @Test
  public void testConvertDelivery() throws AdminException {
    Delivery d = Delivery.newCommonDelivery();
    d.setSourceAddress(new Address("+791394"));
    d.setActivePeriodEnd(new Date());
    d.setActivePeriodStart(new Date(0));
    d.setActiveWeekDays(new Delivery.Day[]{Delivery.Day.Fri, Delivery.Day.Sat});
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
    d.setValidityPeriod("1");
    d.setProperty(UserDataConsts.RESTRICTION, "true");
    assertEquals(d, DcpConverter.convert(d.getId(), DcpConverter.convert(d), null));
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
    mi.setDate(DcpConverter.convertDate(date));
    mi.setErrorCode(1179);
    mi.setId(1);
    mi.setIndex(13);
    mi.setText("sms_text");
    mi.setState(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessageState.Failed);
    mi.setUserData("user=data");
    MessageInfo info = DcpConverter.convert(mi, null);
    assertEquals(mi.getAbonent(), info.getAbonent());
    assertEquals(DcpConverter.convertDate(mi.getDate()), info.getDate());
    assertNotNull(info.getErrorCode());
    assertEquals(mi.getErrorCode(), info.getErrorCode().intValue());
    assertEquals(mi.getId(), info.getId());
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
    di.setActivityPeriodEnd(DcpConverter.convertTime(new Date()));
    di.setActivityPeriodStart(DcpConverter.convertTime(new Date(1000003231L)));
    di.setDeliveryId(45);
    di.setEndDate(DcpConverter.convertDate(new Date(System.currentTimeMillis() + 1000000)));
    di.setName("test delivery");
    di.setStartDate(DcpConverter.convertDate(new Date()));
    di.setStatus(DcpConverter.convert(DeliveryStatus.Cancelled));
    di.setUserId("user2");
    DeliveryInfo info = DcpConverter.convert(di);
    assertEquals(DcpConverter.convertTime(info.getActivityPeriodEnd()), di.getActivityPeriodEnd());
    assertEquals(DcpConverter.convertTime(info.getActivityPeriodStart()), di.getActivityPeriodStart());
    assertEquals(DcpConverter.convertDate(info.getStartDate()), di.getStartDate());
    assertEquals(DcpConverter.convertDate(info.getEndDate()), di.getEndDate());
    assertEquals(DcpConverter.convert(info.getStatus()), di.getStatus());
    assertEquals(info.getDeliveryId(), di.getDeliveryId());
    assertEquals(info.getName(), di.getName());
    assertEquals(info.getUserId(), di.getUserId());
  }

  @Test
  public void testConvertDeliveryStatus() {
    for(DeliveryStatus ds : DeliveryStatus.values()) {
      assertEquals(ds, DcpConverter.convert(DcpConverter.convert(ds)));
    }
  }

  @Test
  public void testConvertDeliveryFields() {
    DeliveryFields[] df = new DeliveryFields[]{DeliveryFields.StartDate, DeliveryFields.Status};
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[] res = DcpConverter.convert(df);
    assertArrayEquals(res, new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[]{mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields.StartDate,
        mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields.Status});

  }

  @Test
  public void testConvertMessageFields() {
    MessageFields[] df = new MessageFields[]{MessageFields.Date, MessageFields.ErrorCode};
    mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField[] res = DcpConverter.convert(df);
    assertArrayEquals(res, new mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField[]{mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField.Date,
        mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField.ErrorCode});
  }

  @Test
  public void testConvertDeliveryStat() throws AdminException {
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics stats = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics();
    stats.setDeliveredMessages(12);
    stats.setExpiredMessages(13);
    stats.setFailedMessage(14);
    stats.setNewMessages(15);
    stats.setProcessMessage(16);
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState ds = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState();
    ds.setDate(DcpConverter.convertDate(new Date()));
    ds.setStatus(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus.Active);

    DeliveryStatistics s = DcpConverter.convert(stats, ds);
    assertEquals(s.getFailedMessages(), stats.getFailedMessage());
    assertEquals(s.getProcessMessages(), stats.getProcessMessage());
    assertEquals(s.getDeliveredMessages(), stats.getDeliveredMessages());
    assertEquals(s.getExpiredMessages(), stats.getExpiredMessages());
    assertEquals(s.getNewMessages(), stats.getNewMessages());

    assertEquals(s.getDeliveryState(), DcpConverter.convert(ds));
  }

}
