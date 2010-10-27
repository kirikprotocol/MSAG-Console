package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
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
  private Throwable error=null;
  private Thread thread;
  int current=0;
  int total=0;
  private Locale locale;


  public Throwable getError() {
    return error;
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
  
  public abstract void execute(final Locale locale) throws Exception ;

  public String cancel() {
    if(thread!=null) {      
       thread.interrupt();
    }
    return null;
  }

  public int getState() {
    return state;
  }

  public String start(){
      locale = getLocale();
      thread = new LongOperationThread();
      error=null;
      state=1;
      current=0;
      total=1;
      thread.start();
      return null;
  }

  private class LongOperationThread extends Thread {

    private LongOperationThread() {
      error=null;
    }

    @Override
    public void run() {
      try{
        execute(locale);
        state = 2;
      }
      catch (Exception e){
        e.printStackTrace();
        error = e;
        state = 0;
      }
    }
  }
}