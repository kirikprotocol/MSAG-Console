package mobi.eyeline.smsquiz.beans;


import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Properties;
import java.io.*;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class Options extends SmsQuizBean {

  public static final int RESULT_SAVED = SmsQuizBean.PRIVATE_RESULT + 1;

  private String dbUser;
  private String dbPassword;
  private String dbSource;

  private String replyDirName;
  private int fileCollectorDelay;
  private int fileCollectorPeriod;
  private int fileOpenedLimit;

  private String consoleHost;
  private String consolePort;
  private String consoleUser;
  private String consolePasssword;
  private int connectTimeout;
  private int connectCloserPeriod;

  private String infoSmeStatsDir;
  private int statusCheckerDelay;
  private int statusCheckerPeriod;

  private String quizDir;
  private int dirListenerDelay;
  private int dirListenerPeriod;
  private int quizCollDelay;
  private int quizCollPeriod;
  private String dirResults;
  private String dirWork;

  private int jmxPort;
  private String jmxPassword;
  private String jmxUser;

  private String smppConnHost;
  private int smppConnPort;
  private String smppConnSystemId;
  private String smppConnPassword;
  private String smppSystemType;

  private String mbDone;
  private String mbCancel;

  private boolean initialized = false;

  private String archiveDir;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      try {
        dbSource = getConfig().getString("dbpool.source");
        dbUser = getConfig().getString("dbpool.user");
        dbPassword = getConfig().getString("dbpool.password");

        replyDirName = getConfig().getString("replystats.statsFile_dir");
        fileCollectorDelay = getConfig().getInt("replystats.fileCollector_period");
        fileCollectorPeriod = getConfig().getInt("replystats.fileCollector_delay");
        fileOpenedLimit = getConfig().getInt("replystats.fileCollector_limit");

        consoleHost = getConfig().getString("distribution.smsc_console_host");
        consolePort = getConfig().getString("distribution.smsc_console_port");
        consoleUser = getConfig().getString("distribution.smsc_console_login");
        consolePasssword = getConfig().getString("distribution.smsc_console_password");
        connectTimeout = getConfig().getInt("distribution.smsc_console_connect_timeout");
        connectCloserPeriod = getConfig().getInt("distribution.smsc_console_closer_period");

        infoSmeStatsDir = getConfig().getString("distribution.infosme_stats_dir");
        statusCheckerDelay = getConfig().getInt("distribution.status_checker_delay");
        statusCheckerPeriod = getConfig().getInt("distribution.status_checker_period");

        quizDir = getConfig().getString("quizmanager.dir_quiz");
        dirListenerDelay = getConfig().getInt("quizmanager.listener_delay");
        dirListenerPeriod = getConfig().getInt("quizmanager.listener_period");
        quizCollDelay = getConfig().getInt("quizmanager.collector_delay");
        quizCollPeriod = getConfig().getInt("quizmanager.collector_period");
        dirResults = getConfig().getString("quizmanager.dir_result");
        dirWork = getConfig().getString("quizmanager.dir_work");
        archiveDir = getConfig().getString("quizmanager.dir_archive");

        jmxPort = getConfig().getInt("jmx.port");
        jmxPassword = getConfig().getString("jmx.user");
        jmxUser = getConfig().getString("jmx.password");

        String smppFileName = getConfig().getString("smppConfig");
        initSmppProperties(smppFileName);

      } catch (Exception e) {
        logger.error(e);
        e.printStackTrace();
        return error("smsquiz.error.config_param", e.getMessage());
      }
    }
    return result;
  }

  public void initSmppProperties(String fileName) throws Exception {
    File file = new File(fileName);
    if (!file.exists()) {
      throw new Exception("File with smpp configuration not found: " + file.getAbsolutePath());
    }
    InputStream inputStream = null;
    try {
      inputStream = new FileInputStream(fileName);
      Properties prop = new Properties();
      prop.load(inputStream);
      smppConnHost = prop.getProperty("smpp.connector.smsx.host");
      smppConnPort = Integer.parseInt(prop.getProperty("smpp.connector.smsx.port"));
      smppConnSystemId = prop.getProperty("smpp.connector.smsx.systemId");
      smppConnPassword = prop.getProperty("smpp.connector.smsx.password");
      smppSystemType = prop.getProperty("smpp.connector.smsx.systemType");
    } finally {
      if (inputStream != null) {
        inputStream.close();
      }
    }
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null) {
      mbDone = null;
      return save();
    }
    if (mbCancel != null) {
      mbCancel = null;
      return RESULT_DONE;
    }

    return result;
  }

  private int save() {
    try {
      getConfig().setString("dbpool.source", dbSource);
      getConfig().setString("dbpool.user", dbUser);
      getConfig().setString("dbpool.password", dbPassword);

      getConfig().setString("replystats.statsFile_dir", replyDirName);
      getConfig().setInt("replystats.fileCollector_period", fileCollectorDelay);
      getConfig().setInt("replystats.fileCollector_delay", fileCollectorPeriod);
      getConfig().setInt("replystats.fileCollector_limit", fileOpenedLimit);

      getConfig().setString("distribution.smsc_console_host", consoleHost);
      getConfig().setString("distribution.smsc_console_port", consolePort);
      getConfig().setString("distribution.smsc_console_login", consoleUser);
      getConfig().setString("distribution.smsc_console_password", consolePasssword);
      getConfig().setInt("distribution.smsc_console_connect_timeout", connectTimeout);
      getConfig().setInt("distribution.smsc_console_closer_period", connectCloserPeriod);

      getConfig().setString("distribution.infosme_stats_dir", infoSmeStatsDir);
      getConfig().setInt("distribution.status_checker_delay", statusCheckerDelay);
      getConfig().setInt("distribution.status_checker_period", statusCheckerPeriod);

      getConfig().setString("quizmanager.dir_quiz", quizDir);
      getConfig().setInt("quizmanager.listener_delay", dirListenerDelay);
      getConfig().setInt("quizmanager.listener_period", dirListenerPeriod);
      getConfig().setInt("quizmanager.collector_delay", quizCollDelay);
      getConfig().setInt("quizmanager.collector_period", quizCollPeriod);
      getConfig().setString("quizmanager.dir_result", dirResults);
      getConfig().setString("quizmanager.dir_work", dirWork);
      getConfig().setString("quizmanager.dir_archive", archiveDir);

      getConfig().setInt("jmx.port", jmxPort);
      getConfig().setString("jmx.user", jmxPassword);
      getConfig().setString("jmx.password", jmxUser);

      storeSmppProperties(getConfig().getString("smppConfig"));
      config.save();

    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      return error("smsquiz.error.config_param", e.getMessage());
    }
    //getInfoSmeContext().setChangedOptions(true);
    return RESULT_SAVED;
  }

  private void storeSmppProperties(String fileName) throws Exception {
    File file = new File(fileName);
    if (!file.exists()) {
      throw new Exception("File with smpp configuration not found!");
    }
    InputStream inputStream = null;
    OutputStream outputStream = null;
    try {
      inputStream = new FileInputStream(fileName);
      Properties prop = new Properties();
      prop.load(inputStream);
      prop.setProperty("smpp.connector.smsx.host", smppConnHost);
      prop.setProperty("smpp.connector.smsx.port", Integer.toString(smppConnPort));
      prop.setProperty("smpp.connector.smsx.systemId", smppConnSystemId);
      prop.setProperty("smpp.connector.smsx.password", smppConnPassword);
      if (smppSystemType == null) {
        smppSystemType = "";
      }
      prop.setProperty("smpp.connector.smsx.systemType", smppSystemType);
      outputStream = new FileOutputStream(fileName);
      prop.store(outputStream, "");
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      throw e;
    } finally {
      if (inputStream != null) {
        try {
          inputStream.close();
        } catch (Exception e) {
          logger.error("Can't close inputStream", e);
        }
      }
      if (outputStream != null) {
        try {
          outputStream.close();
        } catch (Exception e) {
          logger.error("Can't close outputStream", e);
        }
      }
    }

  }


  public String getDbUser() {
    return dbUser;
  }

  public void setDbUser(String dbUser) {
    this.dbUser = dbUser;
  }

  public String getDbPassword() {
    return dbPassword;
  }

  public void setDbPassword(String dbPassword) {
    this.dbPassword = dbPassword;
  }

  public String getDbSource() {
    return dbSource;
  }

  public void setDbSource(String dbSource) {
    this.dbSource = dbSource;
  }


  public String getReplyDirName() {
    return replyDirName;
  }

  public void setReplyDirName(String replyDirName) {
    this.replyDirName = replyDirName;
  }


  public String getFileCollectorDelay() {
    return Integer.toString(fileCollectorDelay);
  }

  public void setFileCollectorDelay(String fileCollectorDelay) {
    try {
      this.fileCollectorDelay = Integer.parseInt(fileCollectorDelay);
    } catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + fileCollectorDelay);
      e.printStackTrace();
    }
  }

  public String getFileCollectorPeriod() {
    return Integer.toString(fileCollectorPeriod);
  }

  public void setFileCollectorPeriod(String fileCollectorPeriod) {
    try {
      this.fileCollectorPeriod = Integer.parseInt(fileCollectorPeriod);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + fileCollectorPeriod);
      e.printStackTrace();
    }
  }

  public String getFileOpenedLimit() {
    return Integer.toString(fileOpenedLimit);
  }

  public void setFileOpenedLimit(String fileOpenedLimit) {
    try {
      this.fileOpenedLimit = Integer.parseInt(fileOpenedLimit);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + fileOpenedLimit);
      e.printStackTrace();
    }
  }

  public String getConsoleHost() {
    return consoleHost;
  }

  public void setConsoleHost(String consoleHost) {
    this.consoleHost = consoleHost;
  }

  public String getConsolePort() {
    return consolePort;
  }

  public void setConsolePort(String consolePort) {
    this.consolePort = consolePort;
  }

  public String getConsoleUser() {
    return consoleUser;
  }

  public void setConsoleUser(String consoleUser) {
    this.consoleUser = consoleUser;
  }

  public String getConsolePasssword() {
    return consolePasssword;
  }

  public void setConsolePasssword(String consolePasssword) {
    this.consolePasssword = consolePasssword;
  }

  public String getConnectTimeout() {
    return Integer.toString(connectTimeout);
  }

  public void setConnectTimeout(String connectTimeout) {
    try {
      this.connectTimeout = Integer.parseInt(connectTimeout);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + connectTimeout);
      e.printStackTrace();
    }
  }

  public String getConnectCloserPeriod() {
    return Integer.toString(connectCloserPeriod);
  }

  public void setConnectCloserPeriod(String connectCloserPeriod) {
    try {
      this.connectCloserPeriod = Integer.parseInt(connectCloserPeriod);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + connectCloserPeriod, e);
      e.printStackTrace();
    }
  }

  public String getInfoSmeStatsDir() {
    return infoSmeStatsDir;
  }

  public void setInfoSmeStatsDir(String infoSmeStatsDir) {
    this.infoSmeStatsDir = infoSmeStatsDir;
  }

  public String getStatusCheckerDelay() {
    return Integer.toString(statusCheckerDelay);
  }

  public void setStatusCheckerDelay(String statusCheckerDelay) {
    try {
      this.statusCheckerDelay = Integer.parseInt(statusCheckerDelay);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + statusCheckerDelay, e);
      e.printStackTrace();
    }
  }

  public String getStatusCheckerPeriod() {
    return Integer.toString(statusCheckerPeriod);
  }

  public void setStatusCheckerPeriod(String statusCheckerPeriod) {
    try {
      this.statusCheckerPeriod = Integer.parseInt(statusCheckerPeriod);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + statusCheckerPeriod, e);
      e.printStackTrace();
    }
  }

  public String getQuizDir() {
    return quizDir;
  }

  public void setQuizDir(String quizDir) {
    this.quizDir = quizDir;
  }

  public String getDirListenerDelay() {
    return Integer.toString(dirListenerDelay);
  }

  public void setDirListenerDelay(String dirListenerDelay) {
    try {
      this.dirListenerDelay = Integer.parseInt(dirListenerDelay);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + dirListenerDelay, e);
      e.printStackTrace();
    }
  }

  public String getDirListenerPeriod() {
    return Integer.toString(dirListenerPeriod);
  }

  public void setDirListenerPeriod(String dirListenerPeriod) {
    try {
      this.dirListenerPeriod = Integer.parseInt(dirListenerPeriod);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + dirListenerPeriod, e);
      e.printStackTrace();
    }
  }

  public String getQuizCollDelay() {
    return Integer.toString(quizCollDelay);
  }

  public void setQuizCollDelay(String quizCollDelay) {
    try {
      this.quizCollDelay = Integer.parseInt(quizCollDelay);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + quizCollDelay, e);
      e.printStackTrace();
    }
  }

  public String getQuizCollPeriod() {
    return Integer.toString(quizCollPeriod);
  }

  public void setQuizCollPeriod(String quizCollPeriod) {
    try {
      this.quizCollPeriod = Integer.parseInt(quizCollPeriod);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + quizCollPeriod, e);
      e.printStackTrace();
    }
  }

  public String getDirResults() {
    return dirResults;
  }

  public void setDirResults(String dirResults) {
    this.dirResults = dirResults;
  }

  public String getDirWork() {
    return dirWork;
  }

  public void setDirWork(String dirWork) {
    this.dirWork = dirWork;
  }

  public String getJmxPort() {
    return Integer.toString(jmxPort);
  }

  public void setJmxPort(String jmxPort) {
    try {
      this.jmxPort = Integer.parseInt(jmxPort);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + jmxPort, e);
      e.printStackTrace();
    }
  }

  public String getJmxPassword() {
    return jmxPassword;
  }

  public void setJmxPassword(String jmxPassword) {
    this.jmxPassword = jmxPassword;
  }

  public String getJmxUser() {
    return jmxUser;
  }

  public void setJmxUser(String jmxUser) {
    this.jmxUser = jmxUser;
  }

  public String getSmppConnHost() {
    return smppConnHost;
  }

  public void setSmppConnHost(String smppConnHost) {
    this.smppConnHost = smppConnHost;
  }

  public String getSmppConnPort() {
    return Integer.toString(smppConnPort);
  }

  public void setSmppConnPort(String smppConnPort) {
    try {
      this.smppConnPort = Integer.parseInt(smppConnPort);
    }
    catch (NumberFormatException e) {
      logger.error("Can't parse int value: " + jmxPort, e);
      e.printStackTrace();
    }
  }

  public String getSmppConnSystemId() {
    return smppConnSystemId;
  }

  public void setSmppConnSystemId(String smppConnSystemId) {
    this.smppConnSystemId = smppConnSystemId;
  }

  public String getSmppConnPassword() {
    return smppConnPassword;
  }

  public void setSmppConnPassword(String smppConnPassword) {
    this.smppConnPassword = smppConnPassword;
  }

  public String getSmppSystemType() {
    return smppSystemType;
  }

  public void setSmppSystemType(String smppSystemType) {
    this.smppSystemType = smppSystemType;
  }

  public String getMbDone() {
    return mbDone;
  }

  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public String getArchiveDir() {
    return archiveDir;
  }

  public void setArchiveDir(String archiveDir) {
    this.archiveDir = archiveDir;
  }
}
