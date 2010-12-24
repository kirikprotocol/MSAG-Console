package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.11.2010
 * Time: 12:21:36
 */
public interface ContentProviderContext {

  FileSystem getFileSystem();

  // todo зависимость от юзеров надо убрать
  List<User> getUsers();

  User getUser(String userName);

  Region getRegion(Address ab);


  Delivery createDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException;

  Delivery getDelivery(String login, String password, int deliveryId) throws AdminException;

  void activateDelivery(String login, String password, int deliveryId) throws AdminException;

  void dropDelivery(String username, String password, int deliveryId) throws AdminException;

  void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException;



  void addMessages(String login, String password, DataSource<Message> messageSource, int deliveryId) throws AdminException;

  void getMessagesStates(String login, String password, MessageFilter filter, int deliveryId, Visitor<Message> visitor) throws AdminException;

  DeliveryChangesDetector getDeliveryChangesDetector();
}
