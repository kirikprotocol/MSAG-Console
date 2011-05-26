package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Билдер для рассылок
 *
 * @author Artem Snopkov
 */
class SingleDeliveryBuilder implements DeliveryBuilder {

  private static final Logger logger = Logger.getLogger(SingleDeliveryBuilder.class);

  private File file;
  private boolean singleText;
  private User owner;
  private Configuration config;

  private int maximum;
  private int current;
  private Integer deliveryId;
  private int processed;
  private boolean canceled;

  /**
   * Конструктор
   * @param file файл со списком номеров и, возможно, текстами
   * @param singleText true, если файл содержит только номера
   * @param owner владелец рассылки
   * @param config экземпляр Configuration
   */
  SingleDeliveryBuilder(File file, boolean singleText, User owner, Configuration config) {
    this.file = file;
    this.singleText = singleText;
    this.owner = owner;
    this.config = config;
  }

  private Delivery createSingleTextDelivery(DeliveryPrototype delivery, final BufferedReader r) throws AdminException {
    return config.createSingleTextDelivery(owner.getLogin(), delivery, new DataSource<Address>() {
      public Address next() throws AdminException {
        if (canceled) {
          return null;
        }

        try {
          String line = r.readLine();
          if (line == null)
            return null;

          current += line.length();
          processed++;
          return new Address(line);

        } catch (IOException e) {
          logger.error(e, e);
          throw new DeliveryException("internal_error");
        }
      }
    });
  }

  private Delivery createMultiTextDelivery(DeliveryPrototype delivery, final BufferedReader r) throws AdminException {
    return config.createDelivery(owner.getLogin(), delivery, new DataSource<Message>() {
      public Message next() throws AdminException {
        if (canceled) {
          return null;
        }

        try {
          String line = r.readLine();
          if (line == null)
            return null;

          current += line.length();
          int i = line.indexOf('|');
          String address = line.substring(0, i);
          String text = line.substring(i + 1, line.length());
          Message m = Message.newMessage(new Address(address), text);
          processed++;
          return m;

        } catch (IOException e) {
          logger.error(e, e);
          throw new DeliveryException("internal_error");
        }
      }
    });
  }

  public void createDelivery(DeliveryPrototype proto) throws DeliveryControllerException {
    BufferedReader r = null;

    maximum = (int) file.length();

    try {
      r = new BufferedReader(new InputStreamReader(config.getFileSystem().getInputStream(file)));

      Delivery d;
      if (singleText) {
        d = createSingleTextDelivery(proto, r);
      } else {
        d = createMultiTextDelivery(proto, r);
      }

      deliveryId = d.getId();

      if (!canceled) {
        config.activateDelivery(owner.getLogin(), d.getId());
      } else {
        config.dropDelivery(owner.getLogin(), d.getId());
      }

      current = maximum;
    } catch (Exception e) {
      try {
        removeDelivery();
      } catch (AdminException e1) {
        logger.error(e,e);
      }
      throw new DeliveryControllerException("delivery.creation.error", e);
    } finally {
      if (r != null) {
        try {
          r.close();
        } catch (IOException ignored) {
        }
      }
      removeFile();
    }
  }

  private void removeFile() {
    try {
      config.getFileSystem().delete(file);
    } catch (AdminException ignored) {
    }
  }

  public int getCurrent() {
    return current;
  }

  public int getTotal() {
    return maximum;
  }

  public int getProcessed() {
    return processed;
  }

  public void cancelDeliveryCreation(){
    canceled = true;
  }

  public void removeDelivery() throws AdminException {
    if (deliveryId != null)
      config.dropDelivery(owner.getLogin(), deliveryId);
  }

  public void shutdown() throws AdminException {

  }
}
