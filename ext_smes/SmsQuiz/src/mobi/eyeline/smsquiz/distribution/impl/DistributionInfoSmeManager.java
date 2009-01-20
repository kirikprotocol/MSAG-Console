package mobi.eyeline.smsquiz.distribution.impl;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.smscconsole.*;
import mobi.eyeline.smsquiz.storage.ResultSet;
import org.apache.log4j.Logger;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

@SuppressWarnings({"ResultOfMethodCallIgnored"})
public class DistributionInfoSmeManager implements DistributionManager {

  private static final Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);
  private AbstractDynamicMBean monitor;

  private ConsoleConnPool consoleConnPool;

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
  private int countConn;

  private String workDir;

  private String codeOk;
  private final static String STATUS_COMMAND = "infosme task status";
  private final static String CREATE_COMMAND = "infosme task create";
  private final static String ALTER_COMMAND = "infosme task alter";
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
      consoleTimeout = config.getLong("smsc_console_connect_timeout", 120) * 1000;
      countConn = config.getInt("smsc_console_connections", 10);

      this.workDir = workDir;

      codeOk = "100";
      if (statsDir == null) {
        throw new DistributionException("infosme_stats_dir parameter missed in config file", DistributionException.ErrorCode.ERROR_NOT_INITIALIZED);
      }
      File file = new File(statsDir);
      if (!file.exists()) {
        file.mkdirs();
      }
      ConsoleConnPoolFactory.init(login, password, host, port);
      consoleConnPool = ConsoleConnPoolFactory.getConnectionPool(countConn, consoleTimeout);
    } catch (ConfigException e) {
      logger.error("Unable to init StatsFilesCache", e);
      throw new DistributionException("Unable to init StatsFilesCache", e);
    }

    dirFormat = new SimpleDateFormat(dirPattern);
    fileFormat = new SimpleDateFormat(filePattern);
    dateInCommand = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    monitor = new DistributionManagerMBean(this);
  }

  private String createAbFile(Distribution distr) throws DistributionException {
    File file = new File(workDir);
    if (!file.exists()) {
      file.mkdirs();
    }
    PrintWriter writer = null;
    try {
      file = new File(workDir + File.separator + distr.getTaskName().hashCode() + ".csv");
      writer = new PrintWriter(new BufferedWriter(new FileWriter(file)));
      ResultSet rs = distr.abonents();
      String question = distr.getQuestion().replace(System.getProperty("line.separator"), "\\n");
      while (rs.next()) {
        String msisdn = (String) rs.get();
        writer.print(msisdn);
        writer.print("|");
        writer.print(question);
        writer.println();
      }
      writer.flush();
    } catch (Exception e) {
      logger.error(e, e);
      throw new DistributionException("Unable to modified abonents list: ", e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
    return file.getAbsolutePath();
  }


  public String createDistribution(final Distribution distr) throws DistributionException {
    logger.info("Create distribution...");
    if ((distr == null) || (distr.getDateBegin() == null)
        || (distr.getQuestion() == null) || (distr.getDateEnd() == null) || (distr.getDays() == null)
        || (distr.getTimeBegin() == null) || (distr.getTimeEnd() == null)) {
      logger.error("Some fields of argument are empty");
      throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    ConsoleResponse response;
    ConsoleConnection conn = null;
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
      conn = consoleConnPool.getConnection();
      response = conn.sendCommand(command.toString());
      if ((response != null) && (response.isSuccess()) && (response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if (lines.length > 0) {
          String[] tokens = lines[0].trim().split(" ");
          if (tokens.length < 3) {
            throw new ConsoleException("Wrong response");
          }
          String id = tokens[2];
          if (!id.equals("")) {
            if (logger.isInfoEnabled()) {
              logger.info("Sent distribution task in InfoSme, id: " + id);
            }
            return id;
          }
        }
      }
      logger.error("Wrong response: " + response);
      throw new DistributionException("Wrong response: " + response);
    } catch (Exception e) {
      logger.error("Unable to create distribution", e);
      throw new DistributionException("Unable to create distribution", e);
    } finally {
      if (conn != null)
        conn.close();
    }
  }

  public void alterDistribution(final Distribution distr, final String taskId) throws DistributionException {
    logger.info("Alter distribution...");
    if ((distr == null) || (distr.getDateBegin() == null)
        || (distr.getQuestion() == null) || (distr.getDateEnd() == null) || (distr.getDays() == null)
        || (distr.getTimeBegin() == null) || (distr.getTimeEnd() == null)) {
      logger.error("Some fields of argument are empty");
      throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    ConsoleResponse response;
    ConsoleConnection conn = null;
    try {
      StringBuilder command = new StringBuilder();
      command.append(ALTER_COMMAND);
      command.append(getFormatProp(taskId));
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
      conn = consoleConnPool.getConnection();
      response = conn.sendCommand(command.toString());
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        logger.error("Wrong response: " + response);
        throw new DistributionException("Wrong response: " + response);
      }
    } catch (Exception e) {
      logger.error("Unable to create distribution", e);
      throw new DistributionException("Unable to create distribution", e);
    } finally {
      if (conn != null)
        conn.close();
    }
  }

  public void shutdown() {
    consoleConnPool.shutdown();
    logger.info("DistributionManager shutdowned");
  }

  public void resend(String msisdn, String repeatQuestion, String taskId) throws DistributionException {
    if ((msisdn == null) || (taskId == null)) {
      logger.error("Some arguments are null");
      throw new DistributionException("Some arguments are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    StringBuilder builder = new StringBuilder();
    builder.append(RESEND_COMMAND).append(getFormatProp(taskId)).append(getFormatProp(msisdn)).append(getFormatProp(repeatQuestion));
    String command = builder.toString();
    if (logger.isInfoEnabled()) {
      logger.info("Sending console command: " + command);
    }

    ConsoleConnection conn = null;
    try {
      conn = consoleConnPool.getConnection();
      ConsoleResponse response = conn.sendCommand(command);
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        logger.error("Wrong response: " + response);
        throw new DistributionException("Wrong response: " + response);
      }
    } catch (Exception e) {
      logger.error("Can't send command", e);
      throw new DistributionException("Can't send command", e);
    } finally {
      if (conn != null) {
        conn.close();
      }
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
    ConsoleConnection conn = null;
    try {
      conn = consoleConnPool.getConnection();
      ConsoleResponse response = conn.sendCommand(command);
      if ((response == null) || (!response.isSuccess()) || (!response.getStatus().trim().equals(codeOk))) {
        logger.error("Wrong response: " + response);
        throw new DistributionException("Wrong response: " + response);
      }
    } catch (ConsoleException e) {
      logger.error("Can't send command", e);
      throw new DistributionException("Can't send command", e);
    } finally {
      if (conn != null) {
        conn.close();
      }
    }

  }

  public State getState(String id) throws DistributionException {
    if (logger.isInfoEnabled()) {
      logger.info("Get status begins for id: " + id);
    }

    ConsoleConnection conn = null;

    try {
      StringBuilder command = new StringBuilder();
      command.append(STATUS_COMMAND);
      command.append(getFormatProp(id));

      if (logger.isInfoEnabled()) {
        logger.info("Sending console command: " + command.toString());
      }
      conn = consoleConnPool.getConnection();
      ConsoleResponse response = conn.sendCommand(command.toString());

      if ((response != null) && (response.isSuccess()) && (response.getStatus().trim().equals(codeOk))) {
        String[] lines = response.getLines();
        if (lines.length > 0) {
          String[] tokens = lines[0].trim().split(" ");
          if (tokens.length < 3) {
            throw new ConsoleException("Wrong response");
          }
          String status = tokens[2];
          if (status.equals("true")) {
            return State.GENERATED;
          } else if (status.equals("false")) {
            return State.UNGENERATED;
          } else if (status.equals("error")) {
            return State.ERROR;
          } else {
            logger.error("Unknown status for taskId=" + id + ". Status: " + status);
            throw new DistributionException("Unknown status for taskId=" + id + ". Status: " + status);
          }
        }
      }
      logger.error("Wrong response: " + response);
      throw new DistributionException("Wrong response: " + response);
    } catch (Exception e) {
      logger.error("Error during repair status", e);
      throw new DistributionException("Error during repair status", e);
    } finally {
      if (conn != null) {
        conn.close();
      }
    }
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }

  public ResultSet getStatistics(final String id, final Date startDate, final Date endDate) throws DistributionException {
    if (logger.isDebugEnabled()) {
      logger.debug("Getting stats for id: " + id + " from=" + startDate + " till=" + endDate);
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
    if (logger.isDebugEnabled()) {
      logger.debug("Comparable date:" + calendar.getTime());
    }
    while (calendar.getTime().compareTo(endDate) <= 0) {
      date = calendar.getTime();
      File file = new File(path + File.separator + dirFormat.format(date) + File.separator + fileFormat.format(date) + ".csv");
      if (logger.isDebugEnabled()) {
        logger.debug("Search file:" + file.getAbsolutePath());
      }
      if (file.exists()) {
        files.add(file);
        if (logger.isDebugEnabled()) {
          logger.debug("File added for analysis: " + file.getAbsolutePath());
        }
      } else {
        file = new File(path + File.separator + dirFormat.format(date) + File.separator + fileFormat.format(date) + "processed.csv");
        if (file.exists()) {
          files.add(file);
          if (logger.isDebugEnabled()) {
            logger.debug("File added for analysis: " + file.getAbsolutePath());
          }
        }
      }
      calendar.add(Calendar.HOUR_OF_DAY, 1);
    }

    ResultSet result = new DistributionResultSet(files, startDate, endDate, succDeliveryStatus, dateInFilePattern);
    if (logger.isDebugEnabled()) {
      logger.debug("Getting stats finished for id: " + id);
    }
    return result;
  }

  private String getFormatProp(String prop) {
    StringBuilder builder = new StringBuilder();
    builder.append(" \"").append(prop.replace("\"", "\\\"").replace(System.getProperty("line.separator"), "\\n")).append("\"");
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

  Integer getCountConn() {
    return countConn;
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
 
