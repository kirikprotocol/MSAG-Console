package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

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

  private final Locale locale;

  private final DeliveryBuilder fact;


  public ProcessDeliveryPage(DeliveryPrototype delivery, DeliveryBuilder fact, Locale locale) {
    this.delivery = delivery;
    this.fact = fact;
    this.locale = locale;
    thread.start();
    state = 1;
  }

  public int getProcessed() {
    return fact.getProcessed();
  }

  public String getError() {
    return error;
  }

  public int getCurrent() {
    return fact.getCurrent();
  }


  public int getMaximum() {
    return fact.getTotal();
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  protected void _process() throws Exception {
    fact.createDelivery(delivery);
  }

  public boolean isStoped() {
    return thread != null && thread.stop;
  }

  public int getState() {
    return state;
  }

  public String stop() {
    if (thread != null) {
      fact.cancelDeliveryCreation();
      thread.stop = true;
      thread.interrupt();
    }
    return null;
  }

  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    return new UploadFilePage(config, user);
  }

  public String getPageId() {
    return "DELIVERY_CREATE_PROCESSING";
  }

  public void cancel() {
    try {
      fact.removeDelivery();
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
