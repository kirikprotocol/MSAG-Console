package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

import java.io.File;
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

  User getUser(String login);

  // todo зависимость от регионов надо убрать. Нужно унифицировать процесс создания рассылки в каком-нить одном месте.
  Region getRegion(Address ab);


  Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException;

  Delivery createDeliveryWithSingleTextWithData(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException;

  Delivery getDelivery(String login,  int deliveryId) throws AdminException;

  void activateDelivery(String login,  int deliveryId) throws AdminException;

  void pauseDelivery(String login, int deliveryId) throws AdminException;

  void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException;

  void addMessages(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException;

  void addSingleMessagesWithData(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException;

  void getMessagesStates(String login, MessageFilter filter, int deliveryId, Visitor<Message> visitor) throws AdminException;

  File getFtpUserHomeDir(String login) throws AdminException;

  public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException;

  public void dropDelivery(String login, int deliveryId) throws AdminException;

  public void modifyDelivery(String login, Delivery delivery) throws AdminException;

  public void checkNoRestrictions(String login) throws AdminException;
}
