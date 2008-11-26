package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.replystats.Reply;
import org.apache.log4j.Logger;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class StatsFilesCache {

  private AbstractDynamicMBean monitor;
  private long timeLimit;
  private String datePattern;
  private String timePattern;
  private String fileNamePattern;
  private String dirNamePattern;
  private String replyStatsDir;
  private static final Logger logger = Logger.getLogger(StatsFilesCache.class);

  private ConcurrentHashMap<String, CachedStatsFile> filesMap;
  private SimpleDateFormat fileNameFormat;
  private SimpleDateFormat dirNameFormat;
  private ScheduledExecutorService fileCollectorScheduler;

  public StatsFilesCache(final String configFile) throws FileStatsException {
    long delayFirst;
    long iterationPeriod;
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));

      PropertiesConfig config = new PropertiesConfig(c.getSection("replystats").toProperties("."));
      fileNamePattern = "HH";
      dirNamePattern = "yyyyMMdd";
      timePattern = "HH:mm";
      datePattern = "yyyyMMdd";
      replyStatsDir = config.getString("statsFile_dir", null);
      if (replyStatsDir == null) {
        throw new FileStatsException("statsFile_dir parameter missed in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
      File file = new File(replyStatsDir);
      if (!file.exists()) {
        file.mkdirs();
      }

      delayFirst = config.getLong("fileCollector_delay", 60);
      iterationPeriod = config.getLong("fileCollector_period", 60);
      timeLimit = 1000 * config.getLong("fileCollector_limit", 60);
    } catch (ConfigException e) {
      logger.error("Unable to init StatsFilesCache", e);
      throw new FileStatsException("Unable to init StatsFilesCache", e);
    }
    filesMap = new ConcurrentHashMap<String, CachedStatsFile>();
    fileNameFormat = new SimpleDateFormat(fileNamePattern);
    dirNameFormat = new SimpleDateFormat(dirNamePattern);

    fileCollectorScheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "ReplyFileCollector");
      }
    });

    fileCollectorScheduler.scheduleAtFixedRate(new FileCollector(false), delayFirst, iterationPeriod, java.util.concurrent.TimeUnit.SECONDS);
    monitor = new StatsFilesCacheMBean(this);
  }

  public static void main(String[] args) throws ParseException {
    SimpleDateFormat dateFormat = new SimpleDateFormat("HH");
    System.out.println(new Date(0));
    System.out.println(dateFormat.parse("23"));
    System.out.println(dateFormat.parse("23").getTime());
    Calendar cal = Calendar.getInstance();
    cal.set(Calendar.HOUR_OF_DAY, 0);
    cal.set(Calendar.MINUTE, 0);
    cal.set(Calendar.SECOND, 0);
    cal.set(Calendar.MILLISECOND, 0);
    cal.setTimeInMillis(cal.getTime().getTime() + dateFormat.parse("23").getTime() + Calendar.getInstance().getTimeZone().getRawOffset());
    System.out.println(cal.getTime());
    System.out.println(cal.getTime().getTime());

  }


  public Collection<StatsFile> getFiles(final String da, final Date from, final Date till) throws FileStatsException {
    if (logger.isInfoEnabled()) {
      logger.info("Getting files for da=" + da + " from=" + from + " till=" + till);
    }
    if ((da == null) || (from == null) || (till == null)) {
      throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    StatsFile statsFile;
    Collection<StatsFile> files = new LinkedList<StatsFile>();

    File dir = new File(replyStatsDir + File.separator + da);
    if (!dir.exists()) {
      return files;
    }

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(from);
    resetTillHour(calendar);
    final Date modifedFrom = calendar.getTime();

    if (logger.isInfoEnabled()) {
      logger.info("Modified from: " + modifedFrom);
      logger.info("till: " + till);
    }

    try {
      for (File directory : dir.listFiles()) {
        if (directory.isDirectory()) {
          String dirName = directory.getName();
          Date dirDate = dirNameFormat.parse(dirName);
          if (logger.isInfoEnabled()) {
            logger.info("DirDate: " + dirDate);
          }
          for (File f : directory.listFiles()) {
            if (!f.isFile()) {
              continue;
            }
            String name = f.getName();
            if (name.lastIndexOf(".csv") < 0) {
              continue;
            }
            Date fileDate = fileNameFormat.parse(name.substring(0, name.lastIndexOf(".")));
            if (logger.isInfoEnabled()) {
              logger.info("FileDate: " + fileDate);
            }
            Date date = new Date(dirDate.getTime() + fileDate.getTime() + Calendar.getInstance().getTimeZone().getRawOffset());
            if (logger.isInfoEnabled()) {
              logger.info("Date parsed from file: " + date);
            }
            if ((date.compareTo(till) <= 0) && (date.compareTo(modifedFrom) >= 0)) {
              if ((statsFile = lockupFile(da, date, true)) != null) {
                files.add(statsFile);
                if (logger.isInfoEnabled()) {
                  logger.info("File added for analysis: " + statsFile.getName());
                }
              }

            }
          }
        }
      }
    } catch (ParseException e) {
      logger.error("Can't parse filename", e);
      throw new FileStatsException("Can't parse filename", e);
    }
    return files;
  }

  private void resetTillHour(Calendar calendar) {
    if (calendar != null) {
      calendar.set(Calendar.MINUTE, 0);
      calendar.set(Calendar.SECOND, 0);
      calendar.set(Calendar.MILLISECOND, 0);
    }
  }

  public StatsFile getFile(String da, Date date) throws FileStatsException {
    if ((da == null) || (date == null)) {
      throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    return lockupFile(da, date, false);
  }

  public void shutdown() {
    fileCollectorScheduler.shutdown();
    new FileCollector(true).run();
  }

  private String buildKey(final String dest, final Date date) {
    StringBuilder builder = new StringBuilder();
    return builder.append(dest).append(File.separator).append(dirNameFormat.format(date))
        .append(File.separator).append(fileNameFormat.format(date)).toString();
  }

  private StatsFile lockupFile(final String dest, final Date date, boolean checkExist) {
    CachedStatsFile file;
    String key = buildKey(dest, date);

    if ((file = filesMap.get(key)) == null) {
      String filePath = replyStatsDir + File.separator + key + ".csv";
      if (logger.isInfoEnabled()) {
        logger.info("Search file: " + filePath);
      }
      if (checkExist) {
        File f = new File(filePath);
        if (!f.exists())
          return null;
      }
      file = new CachedStatsFile(dest, filePath);
      CachedStatsFile f1 = filesMap.putIfAbsent(key, file);
      if (f1 != null)
        file = f1;
    }
    return file;
  }

  private class CachedStatsFile implements StatsFile {
    private StatsFileImpl statsFileImpl;
    private String filePath;
    private String da;
    private final Lock lock = new ReentrantLock();
    private boolean isClosed = true;
    private Date lastUsageDate;

    public CachedStatsFile(String da, String filePath) {
      this.da = da;
      this.filePath = filePath;
      lastUsageDate = new Date();
    }

    public void open() throws FileStatsException {
      try {
        lock.lock();
        if (statsFileImpl == null) {
          statsFileImpl = new StatsFileImpl(da, filePath, timePattern, datePattern);
        }
        if (isClosed) {
          statsFileImpl.open();
          isClosed = false;
        }
      } catch (FileStatsException e) {
        lock.unlock();
        throw new FileStatsException(e);
      }
      lastUsageDate = new Date();

    }

    public void add(Reply reply) throws FileStatsException {
      statsFileImpl.add(reply);
    }

    public List<Reply> getReplies(String oa, Date from, Date till) throws FileStatsException {
      return statsFileImpl.getReplies(oa, from, till);
    }

    public String getName() {
      if (statsFileImpl != null) {
        return statsFileImpl.getName();
      }
      return null;
    }

    public void close() {
      lock.unlock();
    }

    private void closeExt() {
      statsFileImpl.close();
      isClosed = true;
    }


    public boolean exist() {
      File file = new File(filePath);
      return file.exists();
    }
  }

  private class FileCollector implements Runnable {

    boolean closeAll = false;

    FileCollector(boolean closeAll) {
      this.closeAll = closeAll;
    }

    public void run() {
      if (logger.isInfoEnabled()) {
        logger.info("FileCollector starts...");
      }


      for (Map.Entry<String, CachedStatsFile> entry : filesMap.entrySet()) {
        CachedStatsFile file = entry.getValue();
        String key = entry.getKey();
        if (!file.isClosed) {
          boolean close;
          if (closeAll) {
            close = closeAll;
          } else {
            close = (System.currentTimeMillis() - file.lastUsageDate.getTime()) > timeLimit;
          }
          if (close) {
            try {
              file.open();
              file.closeExt();
              file.close();
              filesMap.remove(key);
            } catch (FileStatsException exc) {
              logger.error("Error lock the cached file", exc);
            }
          }
        }
      }
      if (logger.isInfoEnabled()) {
        logger.info("FileCollector finishes.");
      }
    }
  }

  public String getOpenedFiles() {
    StringBuilder builder = new StringBuilder("\n");
    for (StatsFile file : filesMap.values()) {
      builder.append(file.getName()).append("\n");
    }
    return builder.substring(1);
  }

  public int countOpenedFiles() {
    if (filesMap != null) {
      return filesMap.values().size();
    } else {
      return 0;
    }
  }

  String getReplyStatsDir() {
    return replyStatsDir.substring(0);
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }

  String getDatePattern() {
    return datePattern;
  }

  void setDatePattern(String datePattern) {
    this.datePattern = datePattern;
  }

  String getTimePattern() {
    return timePattern;
  }

  void setTimePattern(String timePattern) {
    this.timePattern = timePattern;
  }

  void setReplyStatsDir(String replyStatsDir) {
    this.replyStatsDir = replyStatsDir;
  }

  String getFileNamePattern() {
    return fileNamePattern;
  }

}


 
