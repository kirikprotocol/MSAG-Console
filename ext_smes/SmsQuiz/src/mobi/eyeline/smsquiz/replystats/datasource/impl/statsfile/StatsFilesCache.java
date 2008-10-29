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
  private String replyStatsDir;
  private static final Logger logger = Logger.getLogger(StatsFilesCache.class);

  private ConcurrentHashMap<String, CachedStatsFile> filesMap;
  private SimpleDateFormat fileNameFormat;
  private ScheduledExecutorService fileCollectorScheduler;

  public StatsFilesCache(final String configFile) throws FileStatsException {
    long delayFirst;
    long iterationPeriod;
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));

      PropertiesConfig config = new PropertiesConfig(c.getSection("replystats").toProperties("."));
      fileNamePattern = config.getString("statsFile.filename.pattern", "yyyyMMdd");
      timePattern = config.getString("statsFile.time.pattern.in.file", "yyyyMMdd");
      datePattern = config.getString("statsFile.date.pattern.in.file", "НН:mm");
      replyStatsDir = config.getString("statsFile.dir.name", null);
      if (replyStatsDir == null) {
        throw new FileStatsException("dir.name parameter missed in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
      File file = new File(replyStatsDir);
      if (!file.exists()) {
        file.mkdirs();
      }

      delayFirst = config.getLong("fileCollector.time.first.delay", 60);
      iterationPeriod = config.getLong("fileCollector.time.period", 60);
      timeLimit = 1000 * config.getLong("fileCollector.time.limit", 60);
      if (fileNamePattern == null) {
        throw new FileStatsException("statsFile.filename.pattern not found in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
    } catch (ConfigException e) {
      logger.error("Unable to init StatsFilesCache", e);
      throw new FileStatsException("Unable to init StatsFilesCache", e);
    }
    filesMap = new ConcurrentHashMap<String, CachedStatsFile>();
    fileNameFormat = new SimpleDateFormat(fileNamePattern);

    fileCollectorScheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "ReplyFileCollector");
      }
    });

    fileCollectorScheduler.scheduleAtFixedRate(new FileCollector(false), delayFirst, iterationPeriod, java.util.concurrent.TimeUnit.SECONDS);
    monitor = new StatsFilesCacheMBean(this);
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

    File dir = new File(replyStatsDir + "/" + da);
    if (!dir.exists()) {
      return files;
    }

    Calendar calendar = Calendar.getInstance();
    calendar.setTime(from);
    resetTillDay(calendar);
    Date modifedFrom = calendar.getTime();
    calendar.setTime(till);
    resetTillDay(calendar);
    Date modifedTill = calendar.getTime();

    if (logger.isInfoEnabled()) {
      logger.info("Modified from: " + modifedFrom);
      logger.info("Modified till: " + modifedTill);
    }

    for (File f : dir.listFiles()) {
      if (f.isFile()) {
        String name = f.getName();
        if (name.lastIndexOf(".csv") < 0) {
          continue;
        }
        try {
          Date date = fileNameFormat.parse(name.substring(0, name.lastIndexOf(".")));
          if (logger.isInfoEnabled()) {
            logger.info("Date parsed from file: " + date);
          }
          if ((date.compareTo(modifedTill) <= 0) && (date.compareTo(modifedFrom) >= 0)) {
            if ((statsFile = lockupFile(da, calendar.getTime(), true)) != null) {
              files.add(statsFile);
              if (logger.isInfoEnabled()) {
                logger.info("File added for analysis: " + f.getAbsolutePath());
              }
            }
          }
        } catch (ParseException e) {
          logger.error("Can't parse filename", e);
          throw new FileStatsException("Can't parse filename", e);
        }
      }
    }
    return files;
  }

  private void resetTillDay(Calendar calendar) {
    if (calendar != null) {
      calendar.set(Calendar.HOUR_OF_DAY, 0);
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

  private String buildKey(final String da, final Date date) {
    String result = "";
    result += da + '_' + fileNameFormat.format(date);
    return result;
  }

  private StatsFile lockupFile(final String dest, final Date date, boolean checkExist) {
    CachedStatsFile file;
    String key = buildKey(dest, date);

    if ((file = filesMap.get(key)) == null) {
      String filePath = replyStatsDir + '/' + dest + '/' + fileNameFormat.format(date) + ".csv";
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


 
