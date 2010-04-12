package mobi.eyeline.welcomesms.backend;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.AdminException;

import java.io.IOException;
import java.io.File;
import java.util.Collection;

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

  private NetworksManager networksManager;

  private LocationService locationService;

  public static WelcomeSMSContext getInstance(SMSCAppContext appContext) throws AdminException {
    if (instance == null) {
      instance = new WelcomeSMSContext(appContext);
    }
    return instance;
  }

  private WelcomeSMSContext(SMSCAppContext appContext) throws AdminException{
    String serviceFolder = appContext.getHostsManager().getServiceInfo("WelcomeSMS").
        getServiceFolder().getAbsolutePath();
    String configDir = serviceFolder + File.separatorChar+ "conf";
    try{
      this.appContext = appContext;
      resetConfig();
      networksManager = new NetworksManager(configDir, config);
      locationService = new LocationService(new File(configDir, "countries.csv"));
      //todo init

    } catch(Exception e) {
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

 public synchronized Collection getNetworks() {
    return networksManager.getNetworks();
  }

  public synchronized Network getNetwork(int id) {
    return networksManager.getNetwork(id);
  }

  public synchronized void saveNetwork(Network n) throws AdminException{
    try{
      networksManager.saveNetwork(n);
    }catch(AdminException e){
      logger.error(e,e);
      throw e;
    }catch(Exception e) {
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }  
  }

  public synchronized void removeNetworks(Collection ids) throws AdminException {
    try{
      networksManager.removeNetworks(ids);
    }catch(Exception e){
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

  public NetworksManager getNetworksManager() {
    return networksManager;
  }

  public LocationService getLocationService() {
    return locationService;
  }
}
