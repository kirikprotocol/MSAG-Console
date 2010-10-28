package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.context.FacesContext;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

public abstract class LongOperationController extends InformerController {

  protected static final Logger logger = Logger.getLogger(LongOperationController.class);

  private int state=0;
  private AdminException error=null;
  private Thread thread;
  int current=0;
  int total=0;
  private Locale locale;
  private Configuration config;
  private Throwable fatalError;
  private boolean cancelled=false;

  public Throwable getFatalError() {
    return fatalError;
  }



  public String getError() throws Throwable {
    if(fatalError!=null) {
      Throwable t = fatalError;
      fatalError = null;
      throw t;
    }
    if(error!=null) {
      AdminException ex = error;
      error = null;
      return ex.getMessage(getLocale());
    }
    return null;
  }

  public int getCurrent(){
    return current;
  }

  public void setCurrent(int current) {
    this.current = current;
  }

  public  int getTotal() {
    return total;
  }

  public void setCurrentAndTotal(int current, int total) {
    this.current = current;
    this.total = total;
  }
  
  public abstract void execute(Configuration config, final Locale locale) throws Exception ;

  public synchronized String cancel() {
    if(thread!=null) {
       cancelled=true;
       thread.interrupt();
    }
    return null;
  }

  public int getState() {
    return state;
  }

  public boolean isCancelled() {
    return cancelled;
  }

  public synchronized String start(){
      if(thread==null) {
        locale = getLocale();
        config = getConfig();
        thread = new LongOperationThread();
        error=null;
        fatalError=null;
        current=0;
        total=1;
        cancelled=false;
        state=1;
        thread.start();
      }
      return null;
  }

  public synchronized void reset() {
    cancel();
    error=null;
    fatalError=null;
    current=0;
    total=1;
    cancelled=false;
    state=0;
  }

  private class LongOperationThread extends Thread {

    private LongOperationThread() {
      error=null;
    }

    @Override
    public void run() {
      try{
        execute(config,locale);
        if(cancelled) {
          throw new InterruptedException();
        }
        state = 2;
      }
      catch(InterruptedException e) {
        e.printStackTrace();
        state=0;
      }
      catch (AdminException e){
        logger.error("AdminException in execute thread",e);
        e.printStackTrace();
        error = e;
        state=3;
      }
      catch (Throwable e) {
        logger.error("Throwable in execute thread",e);
        e.printStackTrace();
        fatalError = e;
        state=3;
      }
      finally {
        synchronized (this) {
          thread=null;
        }
      }
    }
  }
}