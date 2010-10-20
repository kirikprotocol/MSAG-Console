package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.ReqField;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
class DcpConverter {   //todo

  static Delivery convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo di) {
    return null;
  }

  static mobi.eyeline.informer.admin.delivery.DeliveryInfo convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryListInfo di) {
    return null;
  }

  static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo convert(Delivery d) {
    return null;
  }

  static String convert(Date date) {
    return null;
  }  

  static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus[] convert(DeliveryStatus ... status) {
    return null;
  }
  static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[] convert(DeliveryFields... fields) {
    return null;
  }
  static ReqField[] convert(MessageFields ... fields) {
    return null;
  }
  static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMessageState[] convert(MessageState ... states) {
    return null;
  }

}
