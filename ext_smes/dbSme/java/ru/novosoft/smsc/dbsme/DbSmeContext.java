package ru.novosoft.smsc.dbsme;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by igork
 * Date: Jul 16, 2003
 * Time: 4:50:30 PM
 */
public class DbSmeContext
{
  private static final Map instances = new HashMap();

  public static synchronized DbSmeContext getInstance(SMSCAppContext appContext, String smeId, int adminPort) throws AdminException
  {
    DbSmeContext context = (DbSmeContext) instances.get(smeId);
    if (context == null) {
      context = new DbSmeContext(appContext, smeId, adminPort);
      instances.put(smeId, context);
    }
    return context;
  }


  final private String smeId;
  final private SmeTransport smeTransport;
  private boolean configChanged = false;
  private boolean jobsChanged = false;
  final private Category logger = Category.getInstance(this.getClass());

  private DbSmeContext(SMSCAppContext appContext, String smeId, int adminPort) throws AdminException
  {
    this.smeId = smeId;
    ServiceInfo serviceInfo = null;
    serviceInfo = appContext.getHostsManager().getServiceInfo(this.smeId);
    this.smeTransport = serviceInfo == null ? null : new SmeTransport(serviceInfo, adminPort);
    this.configChanged = false;
    File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");
    if (tempConfigFile.exists()) {
      File origConfigFile = null;
      if (serviceInfo != null) {
        origConfigFile = new File(serviceInfo.getServiceFolder(), "conf/config.xml");
      } else {
        logger.error("Could not get DbSme original config, nested: Could not get service info");
      }
      if (origConfigFile != null && origConfigFile.exists()) {
        if (origConfigFile.length() != tempConfigFile.length()) {
          this.configChanged = true;
          return;
        }
        try {
          InputStream temp = new BufferedInputStream(new FileInputStream(tempConfigFile));
          InputStream orig = new BufferedInputStream(new FileInputStream(origConfigFile));
          for (int readedTemp = temp.read(); readedTemp != -1; readedTemp = temp.read()) {
            if (readedTemp != orig.read()) {
              configChanged = true;
              temp.close();
              orig.close();
              return;
            }
          }
        } catch (Exception e) {
          System.out.println("Couldn't compare origianl and saved configs, nested: " + e.getMessage());
        }
      }
    }
  }

  public boolean isConfigChanged()
  {
    return configChanged;
  }

  public void setConfigChanged(boolean configChanged)
  {
    this.configChanged = configChanged;
  }

  public SmeTransport getSmeTransport()
  {
    return smeTransport;
  }

  public boolean isJobsChanged()
  {
    return jobsChanged;
  }

  public void setJobsChanged(boolean jobsChanged)
  {
    this.jobsChanged = jobsChanged;
  }

  public String getSmeId()
  {
    return smeId;
  }
}
