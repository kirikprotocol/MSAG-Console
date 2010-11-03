package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DataSource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class ProcessDeliveryPage extends InformerController implements CreateDeliveryPage{

  private static final Logger logger = Logger.getLogger(ProcessDeliveryPage.class);

  private int state = 1;

  private final ProcessThread thread;

  private String error;

  private final Delivery delivery;

  private final Configuration config;

  private final Locale locale;

  private final String user;

  private final File tmpFile;

  private int current = 0;

  private int maximum = Integer.MAX_VALUE;

  public ProcessDeliveryPage(Delivery delivery, File tmpFile, Configuration config, Locale locale, String user) {
    this.delivery = delivery;
    this.config = config;
    this.user = user;
    this.locale = locale;
    this.tmpFile = tmpFile;
    thread = new ProcessThread();
    thread.start();
    state=1;
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

  @SuppressWarnings({"EmptyCatchBlock"})
  protected void _process() throws Exception{

    User u = config.getUser(user);

    final BufferedReader[] r = new BufferedReader[]{null};

    maximum = (int)tmpFile.length();

    try{
      r[0] = new BufferedReader(new FileReader(tmpFile));

      if(delivery.getType() == Delivery.Type.SingleText) {
        config.createDelivery(u.getLogin(), u.getPassword(), delivery, new DataSource<String>() {
          public String next() throws AdminException {
            String line;
            try {
              if((line = r[0].readLine()) != null) {
                current += line.length();
                return line;
              }
            } catch (IOException e) {
              logger.error(e,e);
              throw new DeliveryException("internal_error");
            }
            return null;
          }
        });
      }else {
        config.createDelivery(u.getLogin(), u.getPassword(), delivery, new DataSource<Message>() {
          public Message next() throws AdminException {
            String line;
            try {
              if((line = r[0].readLine()) != null) {
                current += line.length();
                String[] s = line.split(",",2);
                Message m = Message.newMessage(s[1]);
                m.setAbonent(new Address(s[0]));
                return m;
              }
            } catch (IOException e) {
              logger.error(e,e);
              throw new DeliveryException("internal_error");
            }
            return null;
          }
        });
      }
      config.activateDelivery(u.getLogin(), u.getPassword(), delivery.getId());
      current = maximum;
    }finally {

      if(r[0] != null) {
        try{
          r[0].close();
        }catch (IOException e){}
      }

    }
  }

  protected boolean isStoped() {
    return thread != null && thread.stop;
  }


  public int getState() {
    return state;
  }

  public String stop() {
    if(thread != null) {
      thread.interrupt();
    }
    return null;
  }

  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }

  public CreateDeliveryPage process(String user, Configuration config) throws AdminException {
    return null;                                                                        //todo
  }

  public String getPageId() {
    return "PROCESSING";
  }

  public void cancel() {
    tmpFile.delete();
    try{
      if(delivery.getId() != null) {
        User u = config.getUser(user);
        config.dropDelivery(u.getLogin(), u.getPassword(), delivery.getId());
      }
    }catch (AdminException e){
      addError(e);
    }
  }

  private class ProcessThread extends Thread {

    private boolean finished = false;

    private boolean stop = false;

    @Override
    public void run() {
      try{
        _process();
      }catch (AdminException e){
        error = e.getMessage(locale);
        logger.warn(e,e);
      }catch (InterruptedException e){
        stop = true;                 //todo
      }catch (Exception e){
        error = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("internal.error");
        logger.error(e,e);
      }finally {
        finished = true;
      }
    }

    private boolean isFinished() {
      return finished;
    }
  }



}
