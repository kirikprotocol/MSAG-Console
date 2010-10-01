package mobi.eyeline.informer.web.controllers.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;
import org.apache.log4j.Logger;
import org.apache.myfaces.trinidad.model.UploadedFile;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.concurrent.atomic.AtomicLong;

/**
 * @author Aleksandr Khalitov
 */
public class BlacklistUploadController extends InformerController{

  private static final Logger logger = Logger.getLogger(BlacklistUploadController.class);

  private int unrecognized;

  private int uploaded;

  private int state = 0;

  private UploadedFile file;

  private UploadThread thread;

  private int current;

  private String error;

  public BlacklistUploadController() {
  }

  public String getError() {
    return error;
  }

  public UploadedFile getFile() {
    return file;
  }

  public void setFile(UploadedFile file) {
    this.file = file;
  }

  public int getUnrecognized() {
    return unrecognized;
  }

  public int getUploaded() {
    return uploaded;
  }


  public int getState() {
    return state;
  }


  public String upload(){
    if(file != null) {
      boolean add = getRequestParameter("file_add") != null;
      thread = new UploadThread(add, file, getUserName(), getLocale());
      thread.start();
      state=1;
    }
    return null;
  }

  public String next() {
    state = 0;
    unrecognized = 0;
    uploaded = 0;
    file = null;
    error = null;
    return "BLACKLIST";
  }

  public String stop() {
    if(thread != null) {
      thread.stop = true;
    }
    return null;
  }

  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }

  public int getCurrent() {
    return thread == null ? 0 : (int)thread.getCurrent();
  }

  public int getMaximum() {
    return thread == null ? 0 : (int)thread.getMaximum();
  }

  private class UploadThread extends Thread {

    private AtomicLong current;

    private AtomicLong maximum;

    private boolean add;

    private UploadedFile file;

    private boolean finished = false;

    private String user;

    private Locale locale;

    private boolean stop = false;

    private UploadThread(boolean add, UploadedFile file, String user, Locale locale) {
      this.add = add;
      this.file = file;
      current = new AtomicLong(0);
      maximum = new AtomicLong(file.getLength());
      this.user = user;
      this.locale = locale;
    }

    @SuppressWarnings({"EmptyCatchBlock"})
    @Override
    public void run() {
      String line;
      List<String> list = new ArrayList<String>(1000);
      BufferedReader reader = null;
      try{
        reader = new BufferedReader(new InputStreamReader(file.getInputStream()));
        while((line = reader.readLine()) != null && !stop) {
          line = line.trim();
          current.addAndGet(line.length());
          if(!Address.validate(line)) {
            unrecognized++;
            continue;
          }
          list.add(line);
          if(list.size() == 1000) {
            if(add) {
              getConfiguration().addInBlacklist(list,user);
            }else {
              getConfiguration().removeFromBlacklist(list, user);
            }
            uploaded+=1000;
            list.clear();
          }
        }
        if(!list.isEmpty() && !stop) {
          int s = list.size();
          if(add) {
            getConfiguration().addInBlacklist(list, user);
          }else {
            getConfiguration().removeFromBlacklist(list,user);
          }
          uploaded+=s;
        }
      }catch (AdminException e){
        error = e.getMessage(locale);
      }catch (Exception e){
        error = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("blaclist.upload.error.text");
      }finally {
        if(reader != null) {
          try{
            reader.close();
          }catch (Exception e){}
        }
        if(!stop) {
          current = maximum;        
        }
        finished = true;
      }
    }

    public boolean isFinished() {
      return finished;
    }

    public int getUnrecognized() {
      return unrecognized;
    }

    public int getUploaded() {
      return uploaded;
    }

    public long getMaximum() {
      return maximum.get();
    }

    public long getCurrent() {
      return current.get();
    }
  }

}
