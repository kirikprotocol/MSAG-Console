package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Конвертор сущностей рассылки
 *
 * @author Aleksandr Khalitov
 */
class DcpConverter {

  private static final Logger logger = Logger.getLogger(DcpConverter.class);

  public static DeliveryMessage convert(Message m) {
    DeliveryMessage result = new DeliveryMessage();
    result.setAbonent(m.getAbonent().getSimpleAddress());
    if (m.getText() != null) {
      result.setText(m.getText());
      result.setMsgType(MessageType.TextMessage);
    } else {
      result.setIndex(m.getGlossaryIndex());
      result.setMsgType(MessageType.GlossaryMessage);
    }

    String userDataStr = convertUserData(m.getProperties());
    if (userDataStr != null)
      result.setUserData(userDataStr);
    return result;
  }

  public static DeliveryMessage[] convert(Collection<Message> dm) {
    if (dm == null) {
      return null;
    }
    DeliveryMessage[] result = new DeliveryMessage[dm.size()];
    int i = 0;
    for (Message m : dm) {
      result[i] = convert(m);
      i++;
    }
    return result;
  }

  public static String convertUserData(Map m) {
    if (m == null || m.isEmpty()) {
      return null;
    }
    StringBuilder sb = new StringBuilder();
    for (Object e1 : m.entrySet()) {
      Map.Entry e = (Map.Entry)e1;      
      sb.append(';').append(e.getKey()).append('=').append(e.getValue());
    }
    return sb.substring(1);
  }

}
