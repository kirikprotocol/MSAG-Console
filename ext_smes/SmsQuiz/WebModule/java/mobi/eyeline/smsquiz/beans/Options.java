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

  private String consoleHost;
  private String consolePort;
  private String consoleUser;
  private String consolePasssword;

  private String infoSmeStatsDir;

  private String quizDir;
  private int dirListenerPeriod;
  private String dirResults;
  private String dirWork;

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

        consoleHost = getConfig().getString("distribution.smsc_console_host");
        consolePort = getConfig().getString("distribution.smsc_console_port");
        consoleUser = getConfig().getString("distribution.smsc_console_login");
        consolePasssword = getConfig().getString("distribution.smsc_console_password");

        infoSmeStatsDir = getConfig().getString("distribution.infosme_stats_dir");

        quizDir = getConfig().getString("quizmanager.dir_quiz");
        dirListenerPeriod = getConfig().getInt("quizmanager.listener_period");
        dirResults = getConfig().getString("quizmanager.dir_result");
        dirWork = getConfig().getString("quizmanager.dir_work");
        archiveDir = getConfig().getString("quizmanager.dir_archive");

        String smppFileName = getConfig().getString("smppConfig");
        initSmppProperties(smppFileName);

      } catch (Exception e) {
        logger.error(e,e);
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

      getConfig().setString("distribution.smsc_console_host", consoleHost);
      getConfig().setString("distribution.smsc_console_port", consolePort);
      getConfig().setString("distribution.smsc_console_login", consoleUser);
      getConfig().setString("distribution.smsc_console_password", consolePasssword);

      getConfig().setString("distribution.infosme_stats_dir", infoSmeStatsDir);

      getConfig().setString("quizmanager.dir_quiz", quizDir);
      getConfig().setInt("quizmanager.listener_period", dirListenerPeriod);
      getConfig().setString("quizmanager.dir_result", dirResults);
      getConfig().setString("quizmanager.dir_work", dirWork);
      getConfig().setString("quizmanager.dir_archive", archiveDir);

      storeSmppProperties(getConfig().getString("smppConfig"));
      config.save();

    } catch (Exception e) {
      logger.error(e,e);
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
      logger.error(e,e);
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

  public String getInfoSmeStatsDir() {
    return infoSmeStatsDir;
  }

  public void setInfoSmeStatsDir(String infoSmeStatsDir) {
    this.infoSmeStatsDir = infoSmeStatsDir;
  }

  public String getQuizDir() {
    return quizDir;
  }

  public void setQuizDir(String quizDir) {
    this.quizDir = quizDir;
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
      logger.error("Can't parse int value: " + smppConnPort, e);
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
