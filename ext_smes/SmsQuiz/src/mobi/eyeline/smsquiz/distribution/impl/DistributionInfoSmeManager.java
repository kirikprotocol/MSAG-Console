package mobi.eyeline.smsquiz.distribution.impl;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.io.*;
import java.text.SimpleDateFormat;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleException;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleResponse;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleClient;
import mobi.eyeline.smsquiz.storage.ResultSet;

public class DistributionInfoSmeManager implements DistributionManager {

  private static final Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);

  private ConcurrentHashMap<String, Status> tasksMap;
  private SmscConsoleClient consoleClient;
  private ScheduledExecutorService scheduledStatusChecker;

  private String statsDir;
  private String dateInFilePattern;
  private String succDeliveryStatus;

  private SimpleDateFormat dirFormat;
  private SimpleDateFormat fileFormat;
  private SimpleDateFormat dateInCommand;
  private String createCommand;
  private String statusCommand;
  private String inprogressStatus;


  private String codeOk;


  public DistributionInfoSmeManager(final String configFile) throws DistributionException {
    String dirPattern;
    String filePattern;
    String successStatus;
    long checkerFirstDleay;
    long checkerPeriod;
    long maxWait;
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));
      PropertiesConfig config = new PropertiesConfig(c.getSection("distribution").toProperties("."));

      statsDir = config.getString("info.sme.stats.dir", null);
      dirPattern = config.getString("info.sme.stats.date.dir.pattern", "yyMMdd");
      filePattern = config.getString("info.sme.stats.time.file.pattern", "HH");
      succDeliveryStatus = config.getString("info.sme.succ.delivery.status", null);
      dateInFilePattern = config.getString("info.sme.date.format.in.file", "yyMMddHHmmss");

      inprogressStatus = config.getString("infosme.inprogress.status");
      successStatus = config.getString("infosme.success.status");
      createCommand = config.getString("smsc.console.command.create");
      statusCommand = config.getString("smsc.console.command.status");
      String host = config.getString("smsc.console.host");
      int port = config.getInt("smsc.console.port");
      String login = config.getString("smsc.console.access.login");
      String password = config.getString("smsc.console.access.password");
      maxWait = config.getLong("infosme.max.wait.creation") * 1000;
      checkerFirstDleay = config.getLong("status.checker.delay.first");
      checkerPeriod = config.getLong("status.checker.period");
      long consoleTimeout = config.getLong("smsc.console.connect.timeout")*1000;
      long closerPeriod = config.getLong("smsc.console.closer.period");


      codeOk = config.getString("smsc.console.code.ok");
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
    DistributionStatusChecker statusChecker = new DistributionStatusChecker(tasksMap, maxWait, statusCommand, codeOk, successStatus, consoleClient);
    scheduledStatusChecker.scheduleAtFixedRate(statusChecker, checkerFirstDleay, checkerPeriod,
        java.util.concurrent.TimeUnit.SECONDS);
  }


  public String createDistribution(final Distribution distr, Runnable task, String errorFile) throws DistributionException {
    logger.info("Create distribution...");
    if ((distr == null) || (distr.getFilePath() == null) || (distr.getDateBegin() == null)
        || (distr.getDateEnd() == null) || (distr.getDays() == null) || (task == null)
        || (distr.getTimeBegin() == null) || (distr.getTimeEnd() == null)) {
      logger.error("Some fields of argument are empty");
      throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    SmscConsoleResponse response = null;
    try {
      File file = new File(distr.getFilePath());
      String fileName = file.getAbsolutePath();

      StringBuilder command = new StringBuilder();
      command.append(createCommand);
      command.append(getFormatProp(fileName));
      command.append(getFormatProp(dateInCommand.format(distr.getDateBegin())));
      command.append(getFormatProp(dateInCommand.format(distr.getDateEnd())));
      command.append(getFormatProp(formatCal(distr.getTimeBegin())));
      command.append(getFormatProp(formatCal(distr.getTimeEnd())));
      command.append(getFormatProp(distr.getDays(",")));
      command.append(getFormatProp(Boolean.toString(distr.isTxmode())));
      command.append(getFormatProp(distr.getSourceAddress()));

      if(logger.isInfoEnabled()) {
        logger.info("Sending console command: "+command.toString());
      }
      response = consoleClient.sendCommand(command.toString());
      if((response!=null)&&(response.isSuccess())&&(response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if(lines.length>0){
          String[] tokens =lines[0].trim().split(" ");
          if(tokens.length<3) {
            throw new SmscConsoleException("Wrong response");
          }
          String id = tokens[2];
          if(!id.equals("")) {
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

  public String repairStatus(String id, String errorFile, Runnable task, Distribution distribution) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Repair status begins for id: " + id);
    }

    try {
      StringBuilder command = new StringBuilder();
      command.append(statusCommand);
      command.append(getFormatProp(id));

      if(logger.isInfoEnabled()) {
        logger.info("Sending console command: "+command.toString());
      }

      SmscConsoleResponse response = consoleClient.sendCommand(command.toString());
      if((response!=null)&&(response.isSuccess())&&(response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if(lines.length>0){
          String[] tokens =lines[0].trim().split(" ");
          if(tokens.length<3) {
            throw new SmscConsoleException("Wrong response");
          }
          String status = tokens[2];
          if (status.equals("")) {
            id = createDistribution(distribution, task, errorFile);
          } else {
            if (status.equals(inprogressStatus)) {
              tasksMap.put(id, new Status(task, errorFile));
            }
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

  public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Getting stats for id: " + id);
    }
    if ((id == null) || (startDate == null) || (endDate == null)) {
      throw new DistributionException();
    }
    Calendar calendar = Calendar.getInstance();
    calendar.setTime(startDate);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    String path = statsDir + "/" + id;
    Date date;
    List<File> files = new LinkedList<File>();

    while (calendar.getTime().compareTo(endDate) <= 0) {
      date = calendar.getTime();
      File file = new File(path + "/" + dirFormat.format(date) + "/" + fileFormat.format(date) + ".csv");
      if (file.exists()) {
        files.add(file);
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
      if(hour<10) {
        builder.append(0);
      }
      builder.append(hour).append(":");
      if(minute<10) {
        builder.append(0);
      }
      builder.append(minute);
      res = builder.toString();
    }
    return res;
  }
}
 
