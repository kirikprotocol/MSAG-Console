package ru.novosoft.smsc.mtsmsme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.02.2005
 * Time: 16:49:50
 * To change this template use File | Settings | File Templates.
 */
public class MTSMSmeContext
{
  private static final Map instances = new HashMap();

  public static synchronized MTSMSmeContext getInstance(final SMSCAppContext appContext, final String smeId)
      throws AdminException, ParserConfigurationException, IOException, SAXException
  {
    MTSMSmeContext instance = (MTSMSmeContext) instances.get(smeId);
    if (null == instance) {
      instance = new MTSMSmeContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }


  private final SMSCAppContext appContext;
  private Config config = null;

  private String smeId = "MTSMSme";

  private MTSMSmeContext(final SMSCAppContext appContext, final String smeId)
      throws AdminException, ParserConfigurationException, SAXException, IOException
  {
    this.smeId = smeId;
    this.appContext = appContext;
    resetConfig();
  }

  public Config getConfig()
  {
    return config;
  }
  public Config loadCurrentConfig()
      throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
                               "conf" + File.separatorChar + "config.xml"));
  }
  public void resetConfig()
      throws AdminException, SAXException, ParserConfigurationException, IOException
  {
    config = loadCurrentConfig();
  }

}
