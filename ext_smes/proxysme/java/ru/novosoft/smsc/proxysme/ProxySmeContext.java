package ru.novosoft.smsc.proxysme;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by igork
 * Date: Aug 5, 2003
 * Time: 3:56:21 PM
 */
public class ProxySmeContext
{
  private static Map instances = new HashMap();

  public static ProxySmeContext getInstance(SMSCAppContext appContext, String proxySmeId) throws AdminException, IOException, ParserConfigurationException, SAXException
  {
    ProxySmeContext instance = (ProxySmeContext) instances.get(proxySmeId);
    if (instance == null)
      instances.put(proxySmeId, instance = new ProxySmeContext(appContext, proxySmeId));

    return instance;
  }


  private SMSCAppContext appContext = null;
  private final String proxySmeId;
  private Config config = null;

  public ProxySmeContext(SMSCAppContext appContext, String proxySmeId) throws IOException, SAXException, AdminException, ParserConfigurationException
  {
    this.appContext = appContext;
    this.proxySmeId = proxySmeId;
    this.config = loadConfig();
  }

  public Config getConfig()
  {
    return config;
  }

  public Config loadConfig() throws AdminException, SAXException, ParserConfigurationException, IOException
  {
    return config = new Config(new File(appContext.getHostsManager().getServiceInfo(proxySmeId).getServiceFolder(),
                                        "conf" + File.separatorChar + "config.xml"));
  }
}
