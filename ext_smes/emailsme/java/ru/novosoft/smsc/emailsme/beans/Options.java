package ru.novosoft.smsc.emailsme.beans;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 16:28:01
 * To change this template use Options | File Templates.
 */
public class Options extends SmeBean
{
  private String smpp_host = "";
  private int smpp_port = 0;
  private int smpp_timeout = 0;
  private String smpp_systemId = "";
  private String smpp_password = "";
  private String smpp_sourceAddress = "";
  private String smpp_serviceType = "";
  private int smpp_protocolId = 0;
  private int smpp_retryTime = 0;

  private String listener_host = "";
  private int listener_port = 0;

  private String smtp_host = "";
  private int smtp_port = 0;

  private String dataSource_type = "";
  private int dataSource_connections = 0;
  private String dataSource_dbInstance = "";
  private String dataSource_dbUserName = "";
  private String dataSource_dbUserPassword = "";

  private String mail_domain = "";
  private int defaults_dailyLimt = 0;

  private String dataSource_jdbc_source = "";
  private String dataSource_jdbc_driver = "";

  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!isInitialized()) {
      try {
        smpp_host = getConfig().getString("smpp.host");
        smpp_port = getConfig().getInt("smpp.port");
        smpp_timeout = getConfig().getInt("smpp.timeout");
        smpp_systemId = getConfig().getString("smpp.systemId");
        smpp_password = getConfig().getString("smpp.password");
        smpp_sourceAddress = getConfig().getString("smpp.sourceAddress");
        smpp_serviceType = getConfig().getString("smpp.serviceType");
        smpp_protocolId = getConfig().getInt("smpp.protocolId");
        smpp_retryTime = getConfig().getInt("smpp.retryTime");

        listener_host = getConfig().getString("listener.host");
        listener_port = getConfig().getInt("listener.port");

        smtp_host = getConfig().getString("smtp.host");
        smtp_port = getConfig().getInt("smtp.port");

        dataSource_type = getConfig().getString("DataSource.type");
        dataSource_connections = getConfig().getInt("DataSource.connections");
        dataSource_dbInstance = getConfig().getString("DataSource.dbInstance");
        dataSource_dbUserName = getConfig().getString("DataSource.dbUserName");
        dataSource_dbUserPassword = getConfig().getString("DataSource.dbUserPassword");

        dataSource_jdbc_source = getConfig().getString("DataSource.jdbc.source");
        dataSource_jdbc_driver = getConfig().getString("DataSource.jdbc.driver");

        mail_domain = getConfig().getString("mail.domain");
        defaults_dailyLimt = getConfig().getInt("defaults.dailyLimit");
      } catch (Exception e) {
        logger.error(e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (getSmeContext().getConnectionPool() == null)
      message("Applied JDBC properties is incorrect");

    if (mbDone != null)
      return done();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int done()
  {
    getConfig().setString("smpp.host", smpp_host);
    getConfig().setInt("smpp.port", smpp_port);
    getConfig().setInt("smpp.timeout", smpp_timeout);
    getConfig().setString("smpp.systemId", smpp_systemId);
    getConfig().setString("smpp.password", smpp_password);
    getConfig().setString("smpp.sourceAddress", smpp_sourceAddress);
    getConfig().setString("smpp.serviceType", smpp_serviceType);
    getConfig().setInt("smpp.protocolId", smpp_protocolId);
    getConfig().setInt("smpp.retryTime", smpp_retryTime);

    getConfig().setString("listener.host", listener_host);
    getConfig().setInt("listener.port", listener_port);

    getConfig().setString("smtp.host", smtp_host);
    getConfig().setInt("smtp.port", smtp_port);

    getConfig().setString("DataSource.type", dataSource_type);
    getConfig().setInt("DataSource.connections", dataSource_connections);
    getConfig().setString("DataSource.dbInstance", dataSource_dbInstance);
    getConfig().setString("DataSource.dbUserName", dataSource_dbUserName);
    getConfig().setString("DataSource.dbUserPassword", dataSource_dbUserPassword);

    getConfig().setString("DataSource.jdbc.source", dataSource_jdbc_source);
    getConfig().setString("DataSource.jdbc.driver", dataSource_jdbc_driver);


    getConfig().setString("mail.domain", mail_domain);
    getConfig().setInt("defaults.dailyLimit", defaults_dailyLimt);

    return RESULT_DONE;
  }

  public String getSmpp_host()
  {
    return smpp_host;
  }

  public void setSmpp_host(String smpp_host)
  {
    this.smpp_host = smpp_host;
  }

  public int getSmpp_portInt()
  {
    return smpp_port;
  }

  public void setSmpp_portInt(int smpp_port)
  {
    this.smpp_port = smpp_port;
  }

  public String getSmpp_port()
  {
    return String.valueOf(smpp_port);
  }

  public void setSmpp_port(String smpp_port)
  {
    try {
      this.smpp_port = Integer.decode(smpp_port).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.port parameter value: \"" + smpp_port + '"', e);
      this.smpp_port = 0;
    }
  }

  public int getSmpp_timeoutInt()
  {
    return smpp_timeout;
  }

  public void setSmpp_timeoutInt(int smpp_timeout)
  {
    this.smpp_timeout = smpp_timeout;
  }

  public String getSmpp_timeout()
  {
    return String.valueOf(smpp_timeout);
  }

  public void setSmpp_timeout(String smpp_timeout)
  {
    try {
      this.smpp_timeout = Integer.decode(smpp_timeout).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.timeout parameter value: \"" + smpp_timeout + '"', e);
      this.smpp_timeout = 0;
    }
  }

  public String getSmpp_systemId()
  {
    return smpp_systemId;
  }

  public void setSmpp_systemId(String smpp_systemId)
  {
    this.smpp_systemId = smpp_systemId;
  }

  public String getSmpp_password()
  {
    return smpp_password;
  }

  public void setSmpp_password(String smpp_password)
  {
    this.smpp_password = smpp_password;
  }

  public String getSmpp_sourceAddress()
  {
    return smpp_sourceAddress;
  }

  public void setSmpp_sourceAddress(String smpp_sourceAddress)
  {
    this.smpp_sourceAddress = smpp_sourceAddress;
  }

  public String getSmpp_serviceType()
  {
    return smpp_serviceType;
  }

  public void setSmpp_serviceType(String smpp_serviceType)
  {
    this.smpp_serviceType = smpp_serviceType;
  }

  public int getSmpp_protocolIdInt()
  {
    return smpp_protocolId;
  }

  public void setSmpp_protocolIdInt(int smpp_protocolId)
  {
    this.smpp_protocolId = smpp_protocolId;
  }

  public String getSmpp_protocolId()
  {
    return String.valueOf(smpp_protocolId);
  }

  public void setSmpp_protocolId(String smpp_protocolId)
  {
    try {
      this.smpp_protocolId = Integer.decode(smpp_protocolId).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.protocolId parameter value: \"" + smpp_protocolId + '"', e);
      this.smpp_protocolId = 0;
    }
  }

  public int getSmpp_retryTimeInt()
  {
    return smpp_retryTime;
  }

  public void setSmpp_retryTimeInt(int smpp_retryTime)
  {
    this.smpp_retryTime = smpp_retryTime;
  }

  public String getSmpp_retryTime()
  {
    return String.valueOf(smpp_retryTime);
  }

  public void setSmpp_retryTime(String smpp_retryTime)
  {
    try {
      this.smpp_retryTime = Integer.decode(smpp_retryTime).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.retryTime parameter value: \"" + smpp_retryTime + '"', e);
      this.smpp_retryTime = 0;
    }
  }

  public String getListener_host()
  {
    return listener_host;
  }

  public void setListener_host(String listener_host)
  {
    this.listener_host = listener_host;
  }

  public int getListener_portInt()
  {
    return listener_port;
  }

  public void setListener_portInt(int listener_port)
  {
    this.listener_port = listener_port;
  }

  public String getListener_port()
  {
    return String.valueOf(listener_port);
  }

  public void setListener_port(String listener_port)
  {
    try {
      this.listener_port = Integer.decode(listener_port).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid listener.port parameter value: \"" + listener_port + '"', e);
      this.listener_port = 0;
    }
  }

  public String getSmtp_host()
  {
    return smtp_host;
  }

  public void setSmtp_host(String smtp_host)
  {
    this.smtp_host = smtp_host;
  }

  public int getSmtp_portInt()
  {
    return smtp_port;
  }

  public void setSmtp_portInt(int smtp_port)
  {
    this.smtp_port = smtp_port;
  }

  public String getSmtp_port()
  {
    return String.valueOf(smtp_port);
  }

  public void setSmtp_port(String smtp_port)
  {
    try {
      this.smtp_port = Integer.decode(smtp_port).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid smtp.port parameter value: \"" + smtp_port + '"', e);
      this.smtp_port = 0;
    }
  }

  public String getDataSource_type()
  {
    return dataSource_type;
  }

  public void setDataSource_type(String dataSource_type)
  {
    this.dataSource_type = dataSource_type;
  }

  public int getDataSource_connectionsInt()
  {
    return dataSource_connections;
  }

  public void setDataSource_connectionsInt(int dataSource_connections)
  {
    this.dataSource_connections = dataSource_connections;
  }

  public String getDataSource_connections()
  {
    return String.valueOf(dataSource_connections);
  }

  public void setDataSource_connections(String dataSource_connections)
  {
    try {
      this.dataSource_connections = Integer.decode(dataSource_connections).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid DataSource.connections parameter value: \"" + dataSource_connections + '"', e);
      this.dataSource_connections = 0;
    }
  }

  public String getDataSource_dbInstance()
  {
    return dataSource_dbInstance;
  }

  public void setDataSource_dbInstance(String dataSource_dbInstance)
  {
    this.dataSource_dbInstance = dataSource_dbInstance;
  }

  public String getDataSource_dbUserName()
  {
    return dataSource_dbUserName;
  }

  public void setDataSource_dbUserName(String dataSource_dbUserName)
  {
    this.dataSource_dbUserName = dataSource_dbUserName;
  }

  public String getDataSource_dbUserPassword()
  {
    return dataSource_dbUserPassword;
  }

  public void setDataSource_dbUserPassword(String dataSource_dbUserPassword)
  {
    this.dataSource_dbUserPassword = dataSource_dbUserPassword;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMail_domain()
  {
    return mail_domain;
  }

  public void setMail_domain(String mail_domain)
  {
    this.mail_domain = mail_domain;
  }

  public int getDefaults_dailyLimtInt()
  {
    return defaults_dailyLimt;
  }

  public void setDefaults_dailyLimtInt(int defaults_dailyLimt)
  {
    this.defaults_dailyLimt = defaults_dailyLimt;
  }

  public String getDefaults_dailyLimt()
  {
    return String.valueOf(defaults_dailyLimt);
  }

  public void setDefaults_dailyLimt(String defaults_dailyLimt)
  {
    try {
      this.defaults_dailyLimt = Integer.decode(defaults_dailyLimt).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid defaults.dailyLimit parameter value: \"" + defaults_dailyLimt + '"', e);
    }
  }

  public String getDataSource_jdbc_source()
  {
    return dataSource_jdbc_source;
  }

  public void setDataSource_jdbc_source(String dataSource_jdbc_source)
  {
    this.dataSource_jdbc_source = dataSource_jdbc_source;
  }

  public String getDataSource_jdbc_driver()
  {
    return dataSource_jdbc_driver;
  }

  public void setDataSource_jdbc_driver(String dataSource_jdbc_driver)
  {
    this.dataSource_jdbc_driver = dataSource_jdbc_driver;
  }
}
