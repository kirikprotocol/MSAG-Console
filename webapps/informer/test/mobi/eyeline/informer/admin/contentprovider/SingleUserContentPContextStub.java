package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
* author: Aleksandr Khalitov
*/
class SingleUserContentPContextStub implements ContentProviderContext {

  private final User user;

  private final TestDeliveryManager deliveryManager;

  private final FileSystem fileSystem;

  protected CpFileFormat cpFileFormat = CpFileFormat.MTS;

  SingleUserContentPContextStub(User user, TestDeliveryManager deliveryManager, FileSystem fileSystem) {
    this.user = user;
    this.fileSystem = fileSystem;
    user.setAllRegionsAllowed(true);
    this.deliveryManager = deliveryManager;
  }

  SingleUserContentPContextStub(User user, TestDeliveryManager deliveryManager, FileSystem fileSystem, CpFileFormat fileFormat) {
    this(user, deliveryManager, fileSystem);
    this.cpFileFormat = fileFormat;
  }


  @Override
  public FileSystem getFileSystem() {
    return fileSystem;
  }

  @Override
  public List<User> getUsers() {
    List<User> us = new ArrayList<User>(1);
    us.add(user);
    return us;
  }

  @Override
  public User getUser(String login) {
    if(!user.getLogin().equals(login)) {
      return null;
    }
    return user;
  }

  @Override
  public Region getRegion(Address ab) {
    return null;
  }

  public void setCpFileFormat(CpFileFormat cpFileFormat) {
    this.cpFileFormat = cpFileFormat;
  }

  @Override
  public CpFileFormat getCpFileFormat() {
    return cpFileFormat;
  }

  @Override
  public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    return deliveryManager.createDeliveryWithIndividualTexts(login, "", delivery, msDataSource);
  }

  @Override
  public Delivery createDeliveryWithSingleTextWithData(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    return deliveryManager.createDeliveryWithSingleTextWithData(login, "", delivery, msDataSource);
  }

  @Override
  public Delivery getDelivery(String login, int deliveryId) throws AdminException {
    return deliveryManager.getDelivery(login, "", deliveryId);
  }

  @Override
  public void activateDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.activateDelivery(login, "", deliveryId);
    deliveryManager.forceActivatePlannedDeliveries();
  }

  public void pauseDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.pauseDelivery(login, "", deliveryId);
  }

  @Override
  public void dropDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.dropDelivery(login, "", deliveryId);
  }

  @Override
  public void modifyDelivery(String login, Delivery delivery) throws AdminException {
    deliveryManager.modifyDelivery(login, "", delivery);
  }

  @Override
  public void checkNoRestrictions(String login) throws AdminException {
    //To change body of implemented methods use File | Settings | File Templates.
  }

  @Override
  public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException {
    delivery.setOwner(login);

    delivery.setSourceAddress(new Address("100"));

    delivery.setDeliveryMode(DeliveryMode.SMS);

    delivery.setPriority(1);

    delivery.setActivePeriodStart(new Time(0,0,0));

    delivery.setActivePeriodEnd(new Time(23,59,59));

    List<Day> days = new ArrayList<Day>(7);
    for (int i=1; i<8;i++) {
      days.add(Day.valueOf(i));
    }
    delivery.setActiveWeekDays(days.toArray(new Day[days.size()]));

  }

  @Override
  public void addMessages(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    deliveryManager.addIndividualMessages(login, "", messageSource, deliveryId);
  }

  @Override
  public void addSingleMessagesWithData(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    deliveryManager.addSingleTextMessagesWithData(login, "", messageSource, deliveryId);
  }

  @Override
  public void getMessagesStates(String login, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    deliveryManager.getMessages(login, "", filter, _pieceSize, visitor);
  }

  @Override
  public File getFtpUserHomeDir(String login) throws AdminException {
    return null;
  }

  @Override
  public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException {
    deliveryManager.getDeliveries(login,"", deliveryFilter, 1000, visitor);
  }
}
