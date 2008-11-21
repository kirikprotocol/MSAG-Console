package mobi.eyeline.smsquiz.distribution.impl;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleClient;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleException;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleResponse;
import mobi.eyeline.smsquiz.storage.ResultSet;
import org.apache.log4j.Logger;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;

public class DistributionInfoSmeManager implements DistributionManager {

  private static final Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);
  private AbstractDynamicMBean monitor;

  private ConcurrentHashMap<String, Status> tasksMap;
  private SmscConsoleClient consoleClient;
  private ScheduledExecutorService scheduledStatusChecker;

  private String statsDir;
  private String dateInFilePattern;
  private String succDeliveryStatus;

  private SimpleDateFormat dirFormat;
  private SimpleDateFormat fileFormat;
  private SimpleDateFormat dateInCommand;

  private String dirPattern;
  private String filePattern;
  private long checkerFirstDelay;
  private long checkerPeriod;
  private String login;
  private String password;
  private String host;
  private int port;
  long consoleTimeout;
  long closerPeriod;

  private String codeOk;
  private final static String STATUS_COMMAND = "infosme task status";
  private final static String CREATE_COMMAND = "infosme task create";
  private final static String RESEND_COMMAND = "infosme task resend";
  private final static String REMOVE_COMMAND = "infosme task remove";

  public DistributionInfoSmeManager(final String configFile) throws DistributionException {

    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));
      PropertiesConfig config = new PropertiesConfig(c.getSection("distribution").toProperties("."));

      statsDir = config.getString("info.sme.stats.dir");
      dirPattern = "yyMMdd";
      filePattern = "HH";
      succDeliveryStatus = "3";
      dateInFilePattern = "yyMMddHHmmss";

      host = config.getString("smsc.console.host");
      port = config.getInt("smsc.console.port");
      login = config.getString("smsc.console.access.login");
      password = config.getString("smsc.console.access.password");
      checkerFirstDelay = config.getLong("status.checker.delay.first", 60);
      checkerPeriod = config.getLong("status.checker.period", 60);
      consoleTimeout = config.getLong("smsc.console.connect.timeout", 60) * 1000;
      closerPeriod = config.getLong("smsc.console.closer.period", 60);


      codeOk = "100";
      if (statsDir == null) {
        throw new DistributionException("info.sme.stats.dir parameter missed in config file", DistributionException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
      if (succDeliveryStatus == null) {
        throw new DistributionException("info.sme.succ.delivery.status parameter missed in config file", DistributionException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
      File file = new File(statsDir);
      if (!file.exists()) {
        file.mkdirs();
      }

      consoleClient = new SmscConsoleClient(login, password, host, port, consoleTimeout, closerPeriod);
    } catch (ConfigException e) {
      logger.error("Unable to init StatsFilesCache", e);
      throw new DistributionException("Unable to init StatsFilesCache", e);
    }

    tasksMap = new ConcurrentHashMap<String, Status>();
    dirFormat = new SimpleDateFormat(dirPattern);
    fileFormat = new SimpleDateFormat(filePattern);
    dateInCommand = new SimpleDateFormat("dd.MM.yyyy HH:mm");

    scheduledStatusChecker = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "DistributionStatusChecker");
      }
    });
    DistributionStatusChecker statusChecker = new DistributionStatusChecker(tasksMap, STATUS_COMMAND, codeOk, consoleClient);
    scheduledStatusChecker.scheduleAtFixedRate(statusChecker, checkerFirstDelay, checkerPeriod,
        java.util.concurrent.TimeUnit.SECONDS);
    monitor = new DistributionManagerMBean(this);
  }


  public String createDistribution(final Distribution distr, Runnable task, String errorFile) throws DistributionException {
    logger.info("Create distribution...");
    if ((distr == null) || (distr.getFilePath() == null) || (distr.getDateBegin() == null)
        || (distr.getDateEnd() == null) || (distr.getDays() == null) || (task == null)
        || (distr.getTimeBegin() == null) || (distr.getTimeEnd() == null)) {
      logger.error("Some fields of argument are empty. Date: ");
      throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    SmscConsoleResponse response;
    try {
      File file = new File(distr.getFilePath());
      String fileName = file.getAbsolutePath();

      StringBuilder command = new StringBuilder();
      command.append(CREATE_COMMAND);
      command.append(getFormatProp(fileName));
      command.append(getFormatProp(dateInCommand.format(distr.getDateBegin())));
      command.append(getFormatProp(dateInCommand.format(distr.getDateEnd())));
      command.append(getFormatProp(formatCal(distr.getTimeBegin())));
      command.append(getFormatProp(formatCal(distr.getTimeEnd())));
      command.append(getFormatProp(distr.getDays(",")));
      command.append(getFormatProp(Boolean.toString(distr.isTxmode())));
      command.append(getFormatProp(distr.getSourceAddress()));

      if (logger.isInfoEnabled()) {
        logger.info("Sending console command: " + command.toString());
      }
      response = consoleClient.sendCommand(command.toString());
      if ((response != null) && (response.isSuccess()) && (response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if (lines.length > 0) {
          String[] tokens = lines[0].trim().split(" ");
          if (tokens.length < 3) {
            throw new SmscConsoleException("Wrong response");
          }
          String id = tokens[2];
          if (!id.equals("")) {
            tasksMap.put(id, new Status(task, errorFile));
            if (logger.isInfoEnabled()) {
              logger.info("Distribution created, id: " + id);
            }
            return id;
          }
        }
      }
      throw new SmscConsoleException("Wrong response");
    } catch (SmscConsoleException e) {
      logger.error("Unable to create distribution", e);
      throw new DistributionException("Unable to create distribution", e);
    }
  }

  public void shutdown() {
    if (scheduledStatusChecker != null) {
      scheduledStatusChecker.shutdown();
    }
    consoleClient.shutdown();
    logger.info("DistributionManager shutdowned");
  }

  public void resend (String msisdn, String taskId) throws DistributionException{
    if((msisdn==null)||(taskId==null)) {
      logger.error("Some arguments are null");
      throw new DistributionException("Some arguments are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    StringBuilder builder = new StringBuilder();
    builder.append(RESEND_COMMAND).append(getFormatProp(taskId)).append(getFormatProp(msisdn));
    String command = builder.toString();
    if (logger.isInfoEnabled()) {
      logger.info("Sending console command: " + command);
    }
    try {
      SmscConsoleResponse response = consoleClient.sendCommand(command);
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        throw new SmscConsoleException("Wrong response");
      }
    } catch (SmscConsoleException e) {
      logger.error("Can't send command",e);
      throw new DistributionException("Can't send command", e);
    }

  }

  public void removeTask (String taskId) throws DistributionException{
    if(taskId==null) {
      logger.error("Some arguments are null");
      throw new DistributionException("Some arguments are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    StringBuilder builder = new StringBuilder();
    builder.append(REMOVE_COMMAND).append(getFormatProp(taskId));
    String command = builder.toString();
    if (logger.isInfoEnabled()) {
      logger.info("Sending console command: " + command);
    }
    try {
      SmscConsoleResponse response = consoleClient.sendCommand(command);
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        throw new SmscConsoleException("Wrong response");
      }
    } catch (SmscConsoleException e) {
      logger.error("Can't send command",e);
      throw new DistributionException("Can't send command", e);
    }

  }  

  public String repairStatus(String id, String errorFile, Runnable task, Distribution distribution) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Repair status begins for id: " + id);
    }

    try {
      StringBuilder command = new StringBuilder();
      command.append(STATUS_COMMAND);
      command.append(getFormatProp(id));

      if (logger.isInfoEnabled()) {
        logger.info("Sending console command: " + command.toString());
      }

      SmscConsoleResponse response = consoleClient.sendCommand(command.toString());
      if ((response != null) && (response.isSuccess()) && (response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if (lines.length > 0) {
          String[] tokens = lines[0].trim().split(" ");
          if (tokens.length < 3) {
            throw new SmscConsoleException("Wrong response");
          }
          String status = tokens[2];
          if (status.equals("")) {
            id = createDistribution(distribution, task, errorFile);
          } else {
              tasksMap.put(id, new Status(task, errorFile));
          }
          if (logger.isInfoEnabled()) {
            logger.info("Repair status ends, new id: " + id);
          }
          return id;
        }
      }
      throw new SmscConsoleException("Wrong response");
    } catch (SmscConsoleException e) {
      logger.error("Error during repair status", e);
      throw new DistributionException("Error during repair status", e);
    }
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }

  public ResultSet getStatistics(final String id, final Date startDate, final Date endDate) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Getting stats for id: " + id + " from=" + startDate + " till=" + endDate);
    }
    if ((id == null) || (startDate == null) || (endDate == null)) {
      throw new DistributionException();
    }
    Calendar calendar = Calendar.getInstance();
    calendar.setTime(startDate);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    String path = statsDir + File.separator + id;
    Date date;
    List<File> files = new LinkedList<File>();
    if(logger.isInfoEnabled()) {
      logger.info("Comparable date:"+calendar.getTime());
    }
    while (calendar.getTime().compareTo(endDate) <= 0) {
      date = calendar.getTime();
      File file = new File(path + File.separator + dirFormat.format(date) + File.separator + fileFormat.format(date) + ".csv");
      if(logger.isInfoEnabled()) {
        logger.info("Search file:"+file.getAbsolutePath());
      }
      if (file.exists()) {
        files.add(file);
        if (logger.isInfoEnabled()) {
          logger.info("File added for analysis: " + file.getAbsolutePath());
        }
      } else{
        file = new File(path + File.separator + dirFormat.format(date) + File.separator + fileFormat.format(date) + "processed.csv");
        if (file.exists()) {
          files.add(file);
          if (logger.isInfoEnabled()) {
            logger.info("File added for analysis: " + file.getAbsolutePath());
          }
        }
      }
      calendar.add(Calendar.HOUR_OF_DAY, 1);      
    }

    ResultSet result = new DistributionResultSet(files, startDate, endDate, succDeliveryStatus, dateInFilePattern);
    if (logger.isInfoEnabled()) {
      logger.info("Getting stats finished for id: " + id);
    }
    return result;
  }

  private String getFormatProp(String prop) {
    StringBuilder builder = new StringBuilder();
    builder.append(" \"").append(prop).append("\"");
    return builder.toString();
  }

  private String formatCal(Calendar cal) {
    String res = null;
    if (cal != null) {
      StringBuilder builder = new StringBuilder();
      int hour = cal.get(Calendar.HOUR_OF_DAY);
      int minute = cal.get(Calendar.MINUTE);
      if (hour < 10) {
        builder.append(0);
      }
      builder.append(hour).append(":");
      if (minute < 10) {
        builder.append(0);
      }
      builder.append(minute);
      res = builder.toString();
    }
    return res;
  }

  String getDirPattern() {
    return dirPattern;
  }

  String getFilePattern() {
    return filePattern;
  }

  Long getCheckerFirstDelay() {
    return checkerFirstDelay;
  }

  Long getCheckerPeriod() {
    return checkerPeriod;
  }

  String getLogin() {
    return login;
  }

  String getPassword() {
    return password;
  }

  String getHost() {
    return host;
  }

  Integer getport() {
    return port;
  }

  String getCodeok() {
    return codeOk;
  }

  String getUngeneratedDistributions() {
    return tasksMap.keySet().toString();
  }

  Integer countUngeneratedDistributions() {
    return tasksMap.keySet().size();
  }

  Long getConsoleTimeout() {
    return consoleTimeout;
  }

  Long getCloserPeriod() {
    return closerPeriod;
  }

  String getStatsDir() {
    return statsDir;
  }

  String getDateInFilePattern() {
    return dateInFilePattern;
  }

  String getSuccDeliveryStatus() {
    return succDeliveryStatus;
  }
}
 
