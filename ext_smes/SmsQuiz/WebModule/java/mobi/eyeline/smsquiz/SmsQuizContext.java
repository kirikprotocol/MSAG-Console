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

import javax.xml.parsers.ParserConfigurationException;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class SmsQuizContext implements SMEAppContext {

  private Config config = null;

  private SMSCAppContext appContext;
  private String smeId = "InfoSme";
  private final static Map instances = new HashMap();

  private int messagesPageSize = 20;
  private int maxMessTotalSize = 1000;

  private int quizesPageSize = 20;
  private int maxQuizTotalSize = 1000;

  public static SmsQuizContext getInstance(SMSCAppContext appContext, String smeId) throws AdminException, SAXException, ParserConfigurationException, IOException {
    SmsQuizContext instance = (SmsQuizContext) instances.get(smeId);
    if (instance == null) {
      instance = new SmsQuizContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  private SmsQuizContext(SMSCAppContext appContext, String smeId) throws AdminException, SAXException, ParserConfigurationException, IOException {
    this.appContext = appContext;
    this.smeId = smeId;
    appContext.registerSMEContext(this);
    resetConfig();
  }

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException {
    config = loadCurrentConfig();
  }

  public Config loadCurrentConfig()
      throws AdminException, IOException, SAXException, ParserConfigurationException {
    System.out.println("Config parent: " + appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder().getAbsolutePath());
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
}
