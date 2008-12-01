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
import mobi.eyeline.smsquiz.quizmanager.DistributionStatusChecker;
import org.apache.log4j.Logger;

import java.io.*;
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

  private SmscConsoleClient consoleClient;

  private String statsDir;
  private String dateInFilePattern;
  private String succDeliveryStatus;

  private SimpleDateFormat dirFormat;
  private SimpleDateFormat fileFormat;
  private SimpleDateFormat dateInCommand;

  private String dirPattern;
  private String filePattern;
  private String login;
  private String password;
  private String host;
  private int port;
  private long consoleTimeout;
  private long closerPeriod;

  private String workDir;

  private String codeOk;
  private final static String STATUS_COMMAND = "infosme task status";
  private final static String CREATE_COMMAND = "infosme task create";
  private final static String RESEND_COMMAND = "infosme task resend";
  private final static String REMOVE_COMMAND = "infosme task remove";

  public DistributionInfoSmeManager(final String configFile, final String workDir) throws DistributionException {
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));
      PropertiesConfig config = new PropertiesConfig(c.getSection("distribution").toProperties("."));

      statsDir = config.getString("infosme_stats_dir");
      dirPattern = "yyMMdd";
      filePattern = "HH";
      succDeliveryStatus = "3";
      dateInFilePattern = "yyMMddHHmmss";

      host = config.getString("smsc_console_host");
      port = config.getInt("smsc_console_port");
      login = config.getString("smsc_console_login");
      password = config.getString("smsc_console_password");
      consoleTimeout = config.getLong("smsc_console_connect_timeout", 60) * 1000;
      closerPeriod = config.getLong("smsc_console_closer_period", 60);

      this.workDir = workDir;

      codeOk = "100";
      if (statsDir == null) {
        throw new DistributionException("infosme_stats_dir parameter missed in config file", DistributionException.ErrorCode.ERROR_NOT_INITIALIZED);
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

    dirFormat = new SimpleDateFormat(dirPattern);
    fileFormat = new SimpleDateFormat(filePattern);
    dateInCommand = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    monitor = new DistributionManagerMBean(this);
  }

  private String createAbFile(Distribution distr) throws DistributionException{
   File file = new File(workDir);
   if(!file.exists()) {
     file.mkdirs();
   }
   PrintWriter writer = null;
    try {
      file = new File(workDir+File.separator+ distr.getTaskName().hashCode()+".csv");
      writer = new PrintWriter(new BufferedWriter(new FileWriter(file)));
      ResultSet rs = distr.abonents();
      String question = distr.getQuestion().replace(System.getProperty("line.separator"), "\\n");
      while (rs.next()) {
        String msisdn = (String)rs.get();
        writer.print(msisdn);
        writer.print("|");
        writer.print(question);
        writer.println();
      }
      writer.flush();
    } catch (Exception e) {
      logger.error(e,e);
      throw new DistributionException("Unable to modified abonents list: ", e);
    }  finally {
      if (writer != null) {
        writer.close();
      }
    }
    return file.getAbsolutePath();
  }


  public String createDistribution(final Distribution distr) throws DistributionException {
    logger.info("Create distribution...");
    if ((distr == null) || (distr.getDateBegin() == null)
        || (distr.getQuestion() == null) ||(distr.getDateEnd() == null) || (distr.getDays() == null)
        || (distr.getTimeBegin() == null) || (distr.getTimeEnd() == null)) {
      logger.error("Some fields of argument are empty");
      throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    SmscConsoleResponse response;
    try {
      String fileName = createAbFile(distr);

      StringBuilder command = new StringBuilder();
      command.append(CREATE_COMMAND);
      command.append(getFormatProp(fileName));
      command.append(getFormatProp(distr.getTaskName()));
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
            if (logger.isInfoEnabled()) {
              logger.info("Distribution task sent in InfoSme, id: " + id);
            }
            return id;
          }
        }
      }
      logger.error("Wrong response: " + response);
      throw new DistributionException("Wrong response: " + response);
    } catch (SmscConsoleException e) {
      logger.error("Unable to create distribution", e);
      throw new DistributionException("Unable to create distribution", e);
    }
  }

  public void shutdown() {
    consoleClient.shutdown();
    logger.info("DistributionManager shutdowned");
  }

  public void resend(String msisdn, String taskId) throws DistributionException {
    if ((msisdn == null) || (taskId == null)) {
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
        logger.error("Wrong response: " + response);
        throw new DistributionException("Wrong response: " + response);
      }
    } catch (SmscConsoleException e) {
      logger.error("Can't send command", e);
      throw new DistributionException("Can't send command", e);
    }

  }

  public void removeDistribution(String distrId) throws DistributionException {
    if (distrId == null) {
      logger.error("Some arguments are null");
      throw new DistributionException("Some arguments are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    StringBuilder builder = new StringBuilder();
    builder.append(REMOVE_COMMAND).append(getFormatProp(distrId));
    String command = builder.toString();
    if (logger.isInfoEnabled()) {
      logger.info("Sending console command: " + command);
    }
    try {
      SmscConsoleResponse response = consoleClient.sendCommand(command);
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        logger.error("Wrong response: " + response);
        throw new DistributionException("Wrong response: " + response);
      }
    } catch (SmscConsoleException e) {
      logger.error("Can't send command", e);
      throw new DistributionException("Can't send command", e);
    }

  }

  public State getState(String id) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Get status begins for id: " + id);
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
          if (status.equals("true")) {
            return State.GENERATED;
          } else if (status.equals("false")){
            return State.UNGENERATED;
          } else if(status.equals("error")) {
            return State.ERROR;
          } else {
            logger.error("Unknown status for taskId="+id+". Status: "+ status);
            throw new DistributionException("Unknown status for taskId="+id+". Status: "+ status);
          }
        }
      }
      logger.error("Wrong response: " + response);
      throw new DistributionException("Wrong response: " + response);
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
    if (logger.isInfoEnabled()) {
      logger.info("Comparable date:" + calendar.getTime());
    }
    while (calendar.getTime().compareTo(endDate) <= 0) {
      date = calendar.getTime();
      File file = new File(path + File.separator + dirFormat.format(date) + File.separator + fileFormat.format(date) + ".csv");
      if (logger.isInfoEnabled()) {
        logger.info("Search file:" + file.getAbsolutePath());
      }
      if (file.exists()) {
        files.add(file);
        if (logger.isInfoEnabled()) {
          logger.info("File added for analysis: " + file.getAbsolutePath());
        }
      } else {
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
    builder.append(" \"").append(prop.replace("\"", "\\\"")).append("\"");
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
 
