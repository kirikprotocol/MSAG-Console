package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.MessageFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;

/**
* @author Aleksandr Khalitov
*/
interface InformerStrategy {

  int countMessages(String user, String password, MessageFilter filter) throws AdminException;

  void getMessagesStates(String user, String password, MessageFilter filter, int pieceSize, Visitor<Message> visitor) throws AdminException;

  Delivery getDelivery(String login, String password, int deliveryId) throws AdminException;

}
