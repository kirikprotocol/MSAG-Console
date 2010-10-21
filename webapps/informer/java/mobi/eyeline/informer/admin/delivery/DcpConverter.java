package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessage;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField;

import java.util.Date;

/**
 * Конвертор сущностей рассылки
 * @author Aleksandr Khalitov
 */
public class DcpConverter {   //todo

  public static Delivery convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo di) {
    return null;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState convert(DeliveryState ds) {
    return null;
  }

  public static MessageInfo convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo di) {
    return null;
  }
  public static DeliveryMessage[] convert(Message ... di) {
    return null;
  }

  public static mobi.eyeline.informer.admin.delivery.DeliveryInfo convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryListInfo di) {
    return null;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo convert(Delivery d) {
    return null;
  }


  public static String convert(Date date) {
    return null;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus[] convert(DeliveryStatus ... status) {
    return null;
  }
  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[] convert(DeliveryFields... fields) {
    return null;
  }
  public static ReqField[] convert(MessageFields ... fields) {
    return null;
  }
  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessageState[] convert(MessageState ... states) {
    return null;
  }

  public static DeliveryStatistics convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics stats, mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState state) {
    return null;
  }
}
