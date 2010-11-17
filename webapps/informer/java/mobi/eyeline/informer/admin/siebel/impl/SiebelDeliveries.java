package mobi.eyeline.informer.admin.siebel.impl;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;

/**
 * @author Aleksandr Khalitov
 */
public interface SiebelDeliveries {

  public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException;

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException;

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException;

  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException;

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException;

  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException;

  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException;

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException;

  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException;

  public void getDefaultDelivery(String user, Delivery delivery) throws AdminException;
}
