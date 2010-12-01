package mobi.eyeline.informer.admin.cdr;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class CdrDaemon implements DeliveryChangeListener{

  public static final String CDR_DIR = "dir";

  private static final Logger logger = Logger.getLogger("CDR");

  private int fileCompletedInterval = 60000;

  private LinkedHashMap<Integer, Delivery> deliveryCache = new LinkedHashMap<Integer, Delivery>(1000) {
    @Override
    protected boolean removeEldestEntry(Map.Entry<Integer, Delivery> eldest) {
      return size()>1000;
    }
  };

  private final FileSystem fs;

  private final File workDir;

  private PrintWriter writer;

  private File cdrOutputDir;

  private CdrDeliveries deliveries;

  private CdrUsers cdrUsers;

  public CdrDaemon(File workDir, File cdrOutputDir, FileSystem fs, CdrDeliveries deliveries, CdrUsers users) throws InitException{
    if(!cdrOutputDir.exists() && !cdrOutputDir.mkdirs())  {
      throw new InitException("Can't create dir: "+ cdrOutputDir.getAbsolutePath());
    }
    this.fs = fs;
    this.cdrOutputDir = cdrOutputDir;
    this.deliveries = deliveries;
    this.cdrUsers = users;
    this.workDir = workDir;
    if(!this.workDir.exists() && !this.workDir.mkdirs()) {
      throw new InitException("Can't create dir: "+ this.workDir.getAbsolutePath());
    }
    try {
      lastDateBeforeCrash = restore(fs, this.workDir, lastEventsBeforeCrash);
    } catch (AdminException e) {
      logger.error(e,e);
      throw new InitException(e);
    } catch (IOException e) {
      logger.error(e,e);
      throw new InitException(e);
    }
  }


  void setFileCompletedInterval(int fileCompletedInterval) {
    this.fileCompletedInterval = fileCompletedInterval;
  }

  private ScheduledExecutorService roller;

  private boolean started = false;

  public void start() {
    if(!started) {
      roller = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable r) {
          return new Thread(r, "CDR-FILE-ROLLER");
        }
      });
      roller.scheduleAtFixedRate(new Runnable() {
        public void run() {
          try{
            roll();
          }catch (Exception e){
            logger.error(e,e);
          }
        }
      }, 60, 60, TimeUnit.SECONDS);
      started = true;
      if(logger.isDebugEnabled()) {
        logger.debug("CDR-DAEMON has been started");
      }
    }
  }

  public boolean istStrated() {
    return started;
  }

  public void stop() {
    if(started) {
      if(roller != null) {
        roller.shutdown();
      }
      if(logger.isDebugEnabled()) {
        logger.debug("CDR-DAEMON has been shutdowned");
      }
    }
  }

  private synchronized Delivery getDelivery(int deliveryId, String user) throws AdminException {
    Delivery d = deliveryCache.get(deliveryId);
    if(d == null) {
      d = deliveries.getDelviery(user, deliveryId);
      if(d != null) {
        deliveryCache.put(deliveryId, d);
      }
    }
    return d;
  }

  public void setCdrOutputDir(File f) throws AdminException{
    if(!f.exists() && !f.mkdirs()) {
      logger.error("Can't create directory: "+f.getAbsolutePath());
      throw new CdrDaemonException("internal_error");
    }
    this.cdrOutputDir = f;
  }

  private void write(ChangeMessageStateEvent e, Delivery d, User u) {
    writer.print(sdf.format(e.getEventDate()));
    writer.print(',');
    writer.print(StringEncoderDecoder.csvEscape(e.getUserId()));
    writer.print(',');
    writer.print(e.getDeliveryId());
    writer.print(',');
    writer.print(e.getMessageId());
    writer.print(',');
    writer.print(d.getSourceAddress().getSimpleAddress());
    writer.print(',');
    writer.print(e.getAddress().getSimpleAddress());
    writer.print(',');
    writer.print(u.getCdrOriginator());
    writer.print(',');
    writer.print(u.getCdrDestination());
    writer.print(',');
    switch (e.getMessageState()) {
      case Delivered : writer.println('D'); break;
      case Failed : writer.println('F'); break;
      case Expired: writer.println('E'); break;
    }
    writer.flush();
  }


  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");

  private String currentDate;
  private File currentFile;


  static String restore(FileSystem fs, File dir, Set<Long> lastEventsBeforeCrash) throws AdminException, IOException {
    File[] files = fs.listFiles(dir, new FileFilter() {
      public boolean accept(File pathname) {
        return pathname.isFile();
      }
    });
    if(files != null && files.length > 0) {
      Arrays.sort(files, new Comparator<File>() {
        public int compare(File o1, File o2) {
          return o1.getName().compareTo(o2.getName());
        }
      });
      File last;
      last = files[files.length - 1];
      byte c = System.getProperty("line.separator").getBytes()[0];
      FileUtils.truncateFile(last, c, 2);
      BufferedReader r = null;
      try{
        r = new BufferedReader(new InputStreamReader(fs.getInputStream(last)));
        String line;
        String date = null;
        Set<Long> lastEvents = new HashSet<Long>();
        while((line = r.readLine()) != null) {
          List<String> decoded = StringEncoderDecoder.csvDecode(line);
          if(date == null || date.compareTo(decoded.get(0)) < 0) {     // new date > old date
            date = decoded.get(0);
            lastEvents.clear();
          }
          lastEvents.add(Long.parseLong(decoded.get(3)));                // if new date > || new date ==
        }
        lastEventsBeforeCrash.addAll(lastEvents);
        if(!last.setLastModified(System.currentTimeMillis())) {
          logger.warn("Can't set last modified date to file: "+last.getAbsolutePath());
        }
        return date;
      }finally {
        if(r != null) {
          try{
            r.close();
          }catch (IOException ignored){}
        }
      }
    }
    return null;
  }

  private String lastDateBeforeCrash;
  private Set<Long> lastEventsBeforeCrash = new HashSet<Long>();

  private final Lock writeLock = new ReentrantLock();

  public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
    User u = cdrUsers.getUser(e.getUserId());
    if(u == null) {
      logger.error("User hasn't been found with id: "+e.getUserId());
      throw new CdrDaemonException("internal_error");
    }
    if(!u.isCreateCDR()) {
      return;
    }

    Delivery d = getDelivery(e.getDeliveryId(), e.getUserId());

    if(d == null) {
      logger.error("Delivery hasn't been found with id: "+e.getDeliveryId());
      throw new CdrDaemonException("internal_error");
    }

    if(!lastEventsBeforeCrash.isEmpty()) {
      if(lastEventsBeforeCrash.contains(e.getMessageId())) {
        return;
      }
      String recordDate = sdf.format(e.getEventDate());
      int compare = lastDateBeforeCrash.compareTo(recordDate);
      if(compare>0) {
        return;
      }else if(compare < 0) {
        lastEventsBeforeCrash.clear();
      }
    }

    String date = sdf.format(e.getEventDate());

    try{
      writeLock.lock();
      if(writer == null || currentDate == null || !currentDate.equals(date)) {
        if(writer != null) {
          writer.close();
        }
        currentDate = date;
        currentFile = new File(workDir, date+".csv");
        writer =  new PrintWriter(new BufferedWriter(
            new OutputStreamWriter(fs.getOutputStream(currentFile, true))));

      }
      write(e, d, u);
    }finally {
      writeLock.unlock();
    }
  }

  void roll() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Start rolling files...");
    }
    File[] files = fs.listFiles(workDir, new FileFilter() {
      public boolean accept(File pathname) {
        return pathname.isFile();
      }
    });
    for(File f : files) {
      if((System.currentTimeMillis() - f.lastModified()) > fileCompletedInterval) {
        try{
          writeLock.lock();
          if(f.getName().equals(currentFile.getName())) {
            writer.close();
            writer = null;
          }
        }finally {
          writeLock.unlock();
        }
        File to = new File(cdrOutputDir, f.getName());
        fs.rename(f, to);
        if(logger.isDebugEnabled()) {
          logger.debug("File has been rolled: '"+f.getAbsolutePath()+ "' to '"+to.getAbsolutePath()+'\'');
        }
      }
    }
  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {}

}

