package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class ProcessDeliveryPage extends InformerController implements CreateDeliveryPage {

  private int state = 1;

  private final ProcessThread thread = new ProcessThread();

  private String error;

  private final DeliveryPrototype delivery;

  private final boolean singleText;

  private Integer deliveryId;

  private final Configuration config;

  private final Locale locale;

  private final String user;

  private final File tmpFile;

  private int current = 0;

  private int maximum = Integer.MAX_VALUE;

  private int processed;

  public ProcessDeliveryPage(DeliveryPrototype delivery, boolean singleText, File tmpFile, Configuration config, Locale locale, String user) {
    this.delivery = delivery;
    this.singleText = singleText;
    this.config = config;
    this.user = user;
    this.locale = locale;
    this.tmpFile = tmpFile;
    thread.start();
    state = 1;
  }

  public int getProcessed() {
    return processed;
  }

  public String getError() {
    return error;
  }

  public int getCurrent() {
    return current;
  }


  public int getMaximum() {
    return maximum;
  }

  private Delivery createSingleTextDelivery(User u, final BufferedReader r) throws AdminException {
    return config.createSingleTextDelivery(u.getLogin(), u.getPassword(), delivery, new DataSource<Address>() {
      public Address next() throws AdminException {
        if (thread.stop) {
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

  private Delivery createMultiTextDelivery(User u, final BufferedReader r) throws AdminException {
    return config.createDelivery(u.getLogin(), u.getPassword(), delivery, new DataSource<Message>() {
      public Message next() throws AdminException {
        if (thread.stop) {
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

  @SuppressWarnings({"EmptyCatchBlock"})
  protected void _process() throws Exception {

    final User u = config.getUser(user);

    BufferedReader r = null;

    maximum = (int) tmpFile.length();

    try {
      r = new BufferedReader(new InputStreamReader(config.getFileSystem().getInputStream(tmpFile)));

      Delivery d;
      if (singleText) {
        d = createSingleTextDelivery(u, r);
      } else {
        d = createMultiTextDelivery(u, r);
      }

      deliveryId = d.getId();

      if (!thread.stop) {
        config.activateDelivery(u.getLogin(), u.getPassword(), d.getId());
        current = maximum;
      }
    } finally {
      if (r != null) {
        try {
          r.close();
        } catch (IOException e) {
        }
      }

    }
  }

  public boolean isStoped() {
    return thread != null && thread.stop;
  }


  public int getState() {
    return state;
  }

  public String stop() {
    if (thread != null) {
      thread.stop = true;
      thread.interrupt();
    }
    return null;
  }

  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if(tmpFile.delete()) {
      logger.error("Can't delete: "+tmpFile.getAbsolutePath());
    }
    return new UploadFilePage(config, user);
  }

  public String getPageId() {
    return "DELIVERY_CREATE_PROCESSING";
  }

  public void cancel() {
    if(tmpFile.delete()) {
      logger.error("Can't delete: "+tmpFile.getAbsolutePath());
    }
    try {
      if (deliveryId != null) {
        User u = config.getUser(user);
        config.dropDelivery(u.getLogin(), u.getPassword(), deliveryId);
      }
    } catch (AdminException e) {
      addError(e);
    }
  }

  private class ProcessThread extends Thread {

    private boolean finished = false;

    private boolean stop = false;

    @Override
    public void run() {
      try {
        _process();
      } catch (AdminException e) {
        error = e.getMessage(locale);
        logger.warn(e, e);
      } catch (InterruptedException e) {
        stop = true;
      } catch (Exception e) {
        error = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("internal.error");
        logger.error(e, e);
      } finally {
        finished = true;
      }
    }

    private boolean isFinished() {
      return finished;
    }
  }


}
