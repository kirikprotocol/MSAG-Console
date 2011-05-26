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
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Создает рассылку, разбитую по нескольким регионам
 *
 * @author Artem Snopkov
 */
class MultiDeliveryBuilder implements DeliveryBuilder {

  private static final Logger logger = Logger.getLogger(MultiDeliveryBuilder.class);


  private final Map<String, File> files;
  private boolean singleText;
  private User deliveriesOwner;
  private Configuration config;

  private int maximum;
  private int current;
  private List<Integer> deliveryIds;
  private int processed;
  private boolean canceled;

  /**
   * Конструктор
   *
   * @param files           Map с ключем, означающим название региона и значением - файлом со списком номеров
   * @param singleText      true, если все рассылки будут с одним текстом, false - с индивидуальными текстами
   * @param deliveriesOwner владелец рассылки
   * @param config          экземпляр Configuration
   */
  MultiDeliveryBuilder(Map<String, File> files, boolean singleText, User deliveriesOwner, Configuration config) {
    this.files = files;
    this.singleText = singleText;
    this.deliveriesOwner = deliveriesOwner;
    this.config = config;
    this.deliveryIds = new ArrayList<Integer>(files.size());
  }

  private Delivery createSingleTextDelivery(DeliveryPrototype delivery, final BufferedReader r) throws AdminException {
    return config.createSingleTextDelivery(deliveriesOwner.getLogin(), delivery, new DataSource<Address>() {
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
    return config.createDelivery(deliveriesOwner.getLogin(), delivery, new DataSource<Message>() {
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

    maximum = 0;
    for (File file : files.values())
      maximum += (int) file.length();

    String initialName = proto.getName();

    try {
      for (Map.Entry<String, File> entry : files.entrySet()) {
        File f = entry.getValue();
        String region = entry.getKey();

        BufferedReader r = null;
        try {
          r = new BufferedReader(new InputStreamReader(config.getFileSystem().getInputStream(f)));

          proto.setName(initialName + " (" + region + ")");

          Delivery d;
          if (singleText) {
            d = createSingleTextDelivery(proto, r);
          } else {
            d = createMultiTextDelivery(proto, r);
          }

          deliveryIds.add(d.getId());

          if (canceled)
            break;

          config.activateDelivery(deliveriesOwner.getLogin(), d.getId());

        } catch (Exception e) {
          try {
            removeDelivery();
          } catch (AdminException e1) {
            logger.error(e, e);
          }
          throw new DeliveryControllerException("delivery.creation.error", e);
        } finally {
          if (r != null) {
            try {
              r.close();
            } catch (IOException ignored) {
            }
          }
        }
      }

    } finally {
      removeFiles();
    }

    current = maximum;

    if (canceled)
      try {
        removeDelivery();
      } catch (AdminException e) {
        logger.error(e, e);
      }
  }

  private void removeFiles() {
    for (File f : files.values()) {
      try {
        config.getFileSystem().delete(f);
      } catch (AdminException ignored) {
      }
    }
  }

  public int getTotal() {
    return maximum;
  }

  public int getCurrent() {
    return current;
  }

  public int getProcessed() {
    return processed;
  }

  public void cancelDeliveryCreation() {
    canceled = true;
  }

  public void removeDelivery() throws AdminException {
    for (int deliveryId : deliveryIds)
      config.dropDelivery(deliveriesOwner.getLogin(), deliveryId);
  }

  public void shutdown() throws AdminException {
    removeDelivery();
    removeFiles();
  }
}
