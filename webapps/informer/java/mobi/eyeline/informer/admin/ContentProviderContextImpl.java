package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.contentprovider.ContentProviderContext;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

import java.util.List;

/**
* @author Artem Snopkov
*/
class ContentProviderContextImpl implements ContentProviderContext {

  private AdminContext context;

  ContentProviderContextImpl(AdminContext context) {
    this.context = context;
  }


  public FileSystem getFileSystem() {
    return this.context.getFileSystem();
  }

  public List<User> getUsers() {
    return this.context.getUsers();
  }

  public User getUser(String userName) {
    return this.context.getUser(userName);  //To change body of implemented methods use File | Settings | File Templates.
  }

  public Region getRegion(Address ab) {
    return this.context.getRegion(ab);
  }

  public Delivery createDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    return this.context.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
  }

  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    return this.context.getDelivery(login, password, deliveryId);
  }

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    this.context.activateDelivery(login, password, deliveryId);
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    this.context.dropDelivery(login, password, deliveryId);
  }

  public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException {
    this.context.copyUserSettingsToDeliveryPrototype(login, delivery);
  }

  public void addMessages(String login, String password, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    this.context.addMessages(login, password, messageSource, deliveryId);
  }

  public void getMessagesStates(String login, String password, MessageFilter filter, int deliveryId, Visitor<Message> visitor) throws AdminException {
    this.context.getMessagesStates(login, password, filter, deliveryId, visitor);
  }
}
