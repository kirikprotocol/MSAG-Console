package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

/**
 * @author Aleksandr Khalitov
 */
public interface SiebelContext {

  public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException;

  public void dropSiebelDelivery(String login, int deliveryId) throws AdminException;

  public void addMessages(String login, DataSource<Message> msDataSource, int deliveryId) throws AdminException;

  public Delivery getDelivery(String login,int deliveryId) throws AdminException;

  public void modifyDelivery(String login, Delivery delivery) throws AdminException;

  public void cancelDelivery(String login, int deliveryId) throws AdminException;

  public void pauseDelivery(String login, int deliveryId) throws AdminException;

  public void activateDelivery(String login,int deliveryId) throws AdminException;

  public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException;

  public void copyUserSettingsToDeliveryPrototype(String user, DeliveryPrototype delivery) throws AdminException;

  public Region getRegion(Address msisdn) throws AdminException;

  public User getUser(String login) throws AdminException;

  public DeliveryChangesDetector getDeliveryChangesDetector();

  public void checkNoRestrictions(String login) throws AdminException;

  public FileSystem getFileSystem();

}
