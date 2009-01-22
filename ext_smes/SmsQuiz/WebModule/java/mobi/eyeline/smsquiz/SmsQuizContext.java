package mobi.eyeline.smsquiz;

import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.io.IOException;
import java.io.File;
import java.util.Map;
import java.util.HashMap;

import org.xml.sax.SAXException;
import org.apache.log4j.Logger;

import javax.xml.parsers.ParserConfigurationException;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class SmsQuizContext implements SMEAppContext {

  private Config config = null;

  private SMSCAppContext appContext;
  private String smeId = "InfoSme";
  private static final String COMMAND_SERVER_PORT_PARAM = "commandserver.port";

  private final static Map instances = new HashMap();

  private int messagesPageSize = 20;
  private int maxMessTotalSize = 1000;
  private int maxRepliesTotalSize = 1000;
  private int maxResultsTotalSize = 1000;

  private int quizesPageSize = 20;
  private int maxQuizTotalSize = 1000;

  private static final Logger logger = Logger.getLogger(SmsQuizContext.class);

  private SmsQuiz smsQuiz;

  public static SmsQuizContext getInstance(SMSCAppContext appContext, String smeId) throws AdminException{
    SmsQuizContext instance = (SmsQuizContext) instances.get(smeId);
    if (instance == null) {
      instance = new SmsQuizContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  private SmsQuizContext(SMSCAppContext appContext, String smeId) throws AdminException{
    try{
      this.appContext = appContext;
      this.smeId = smeId;
      appContext.registerSMEContext(this);
      resetConfig();
      int port = config.getInt(COMMAND_SERVER_PORT_PARAM);
      this.smsQuiz = new SmsQuiz(appContext.getHostsManager().getServiceInfo(this.smeId),"localhost",port);      //todo
    } catch(Exception e) {
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException {
    config = loadCurrentConfig();
  }

  public Config loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
        "conf" + File.separatorChar + "config.xml"));
  }


  public void shutdown() throws IOException {
  }

  public Config getConfig() {
    return config;
  }

  public int getMessagesPageSize() {
    return messagesPageSize;
  }

  public void setMessagesPageSize(int messagesPageSize) {
    this.messagesPageSize = messagesPageSize;
  }

  public int getMaxMessTotalSize() {
    return maxMessTotalSize;
  }

  public void setMaxMessTotalSize(int maxMessTotalSize) {
    this.maxMessTotalSize = maxMessTotalSize;
  }

  public int getQuizesPageSize() {
    return quizesPageSize;
  }

  public void setQuizesPageSize(int quizesPageSize) {
    this.quizesPageSize = quizesPageSize;
  }

  public int getMaxQuizTotalSize() {
    return maxQuizTotalSize;
  }

  public void setMaxQuizTotalSize(int maxQuizTotalSize) {
    this.maxQuizTotalSize = maxQuizTotalSize;
  }

  public int getMaxRepliesTotalSize() {
    return maxRepliesTotalSize;
  }

  public void setMaxRepliesTotalSize(int maxRepliesTotalSize) {
    this.maxRepliesTotalSize = maxRepliesTotalSize;
  }

  public int getMaxResultsTotalSize() {
    return maxResultsTotalSize;
  }

  public void setMaxResultsTotalSize(int maxResultsTotalSize) {
    this.maxResultsTotalSize = maxResultsTotalSize;
  }

  public SmsQuiz getSmsQuiz() {
    return smsQuiz;
  }

}
