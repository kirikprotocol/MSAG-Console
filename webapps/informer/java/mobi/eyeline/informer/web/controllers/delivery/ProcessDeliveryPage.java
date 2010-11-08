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

import java.io.*;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class ProcessDeliveryPage extends InformerController implements CreateDeliveryPage{

  private static final Logger logger = Logger.getLogger(ProcessDeliveryPage.class);

  private int state = 1;

  private final ProcessThread thread = new ProcessThread();

  private String error;

  private final Delivery delivery;

  private final Configuration config;

  private final Locale locale;

  private final String user;

  private final File tmpFile;

  private int current = 0;

  private int maximum = Integer.MAX_VALUE;

  private File blacklist;

  public ProcessDeliveryPage(Delivery delivery, File tmpFile, Configuration config, Locale locale, String user) {
    this.delivery = delivery;
    this.config = config;
    this.user = user;
    this.locale = locale;
    this.tmpFile = tmpFile;
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

    blacklist = new File(config.getWorkDir(), "blacklist_"+user+System.currentTimeMillis());

    User u = config.getUser(user);

    final BufferedReader[] r = new BufferedReader[]{null};
    final PrintWriter[] b = new PrintWriter[]{null};

    maximum = (int)tmpFile.length();

    try{
      r[0] = new BufferedReader(new FileReader(tmpFile));
      b[0] = new PrintWriter(new BufferedWriter(new FileWriter(blacklist)));

      if(delivery.getType() == Delivery.Type.SingleText) {
        config.createSingleTextDelivery(u.getLogin(), u.getPassword(), delivery, new DataSource<Address>() {
          public Address next() throws AdminException {
            if(thread.stop) {
              return null;
            }
            String line;
            try {
              if((line = r[0].readLine()) != null) {
                current += line.length();
                if(config.blacklistContains(line)) {
                  b[0].println(line);
                }
                return new Address(line);
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
            if(thread.stop) {
              return null;
            }
            String line;
            try {
              if((line = r[0].readLine()) != null) {
                current += line.length();
                String[] s = line.split(",",2);
                Message m = Message.newMessage(s[1]);
                m.setAbonent(new Address(s[0]));
                if(config.blacklistContains(s[0])) {
                  b[0].println(s[0]);
                }
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
      if(!thread.stop) {
        config.activateDelivery(u.getLogin(), u.getPassword(), delivery.getId());
        current = maximum;
      }
    }finally {
      if(r[0] != null) {
        try{
          r[0].close();
        }catch (IOException e){}
      }
      if(b[0] != null) {
        b[0].close();
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
    if(thread != null) {
      thread.stop = true;
      thread.interrupt();
    }
    return null;
  }

  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    tmpFile.delete();
    if(blacklist != null) {
      blacklist.delete();
    }
    return new StartPage();
  }

  public String getPageId() {
    return "DELIVERY_CREATE_PROCESSING";
  }

  public void cancel() {
    tmpFile.delete();
    if(blacklist != null) {
      blacklist.delete();
    }
    try{
      if(delivery.getId() != null) {
        User u = config.getUser(user);
        config.dropDelivery(u.getLogin(), u.getPassword(), delivery.getId());
      }
    }catch (AdminException e){
      addError(e);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  @Override
  protected void _download(PrintWriter writer) throws IOException {
    if(blacklist == null || !blacklist.exists()) {
      return;
    }
    BufferedReader r = null;
    try{
      r = new BufferedReader(new FileReader(blacklist));
      String line;
      while((line = r.readLine()) != null) {
        writer.println(line);
      }
    }finally {
      if(r != null) {
        try{
          r.close();
        }catch (IOException e){}
      }
    }           
  }

  public boolean isRenderBL() {
    return blacklist != null && blacklist.length() > 0 && isFinished() && (error == null);
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
        stop = true;
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
