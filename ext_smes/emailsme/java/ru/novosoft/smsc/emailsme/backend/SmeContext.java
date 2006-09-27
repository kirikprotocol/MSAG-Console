package ru.novosoft.smsc.emailsme.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.util.conpool.ConnectionPool;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.Properties;
import java.sql.SQLException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 15:41:27
 * To change this template use Options | File Templates.
 */
public class SmeContext implements SMEAppContext
{
  public static final String SME_ID = "emailsme";

  private static SmeContext instance = null;

  public static SmeContext getInstance(SMSCAppContext appContext)
      throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return instance == null ? instance = new SmeContext(appContext) : instance;
  }

  private final SMSCAppContext appContext;
  private Config config;
  private String sort = "addr";
  private int pageSize = 20;
  private Category logger = Category.getInstance(this.getClass());

  public SmeContext(SMSCAppContext appContext)
      throws IOException, ParserConfigurationException, SAXException, AdminException
  {
    this.appContext = appContext;
    appContext.registerSMEContext(this);
    resetConfig();
  }



  public void shutdown() {
  }

  public void resetConfig() throws AdminException, IOException, ParserConfigurationException, SAXException
  {
    config = loadCurrentConfig();
  }

  public Config getConfig()
  {
    return config;
  }

  public String getSort()
  {
    return sort;
  }

  public void setSort(String sort)
  {
    this.sort = sort;
  }

  public int getPageSize()
  {
    return pageSize;
  }

  public void setPageSize(int pageSize)
  {
    this.pageSize = pageSize;
  }

  public Config loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(SME_ID).getServiceFolder(), "conf" + File.separatorChar + "config.xml"));
  }
}
