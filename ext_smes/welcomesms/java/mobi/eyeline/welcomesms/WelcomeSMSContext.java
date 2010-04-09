package mobi.eyeline.welcomesms;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.AdminException;

import java.io.IOException;
import java.io.File;

import org.xml.sax.SAXException;
import org.apache.log4j.Category;

import javax.xml.parsers.ParserConfigurationException;

/**
 * author: alkhal
 */
public class WelcomeSMSContext {

  private Config config = null;

  private SMSCAppContext appContext;

  private static WelcomeSMSContext instance;

  private static final Category logger = Category.getInstance(WelcomeSMSContext.class);

  public static WelcomeSMSContext getInstance(SMSCAppContext appContext) throws AdminException {
    if (instance == null) {
      instance = new WelcomeSMSContext(appContext);
    }
    return instance;
  }

  private WelcomeSMSContext(SMSCAppContext appContext) throws AdminException{
    try{
      this.appContext = appContext;
      resetConfig();

      //todo init

    } catch(Exception e) {
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException {
    config = loadCurrentConfig();
  }

  public Config loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException {
    File f = new File(appContext.getHostsManager().getServiceInfo("WelcomeSMS").getServiceFolder(),
        "conf" + File.separatorChar + "config.xml");
    return new Config(f);
  }


  public void shutdown() throws IOException {
  }

  public Config getConfig() {
    return config;
  }
}
