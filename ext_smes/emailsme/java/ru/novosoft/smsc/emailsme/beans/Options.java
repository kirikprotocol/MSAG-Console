package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 16:28:01
 * To change this template use Options | File Templates.
 */
public class Options extends SmeBean
{
  private String store_dir = "";

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

  private String admin_host = "";
  private int admin_port = 0;
  private boolean admin_allow_gsm_2_email_without_profile = false;
  private boolean admin_allow_email_2_gsm_without_profile = false;
  private String admin_default_limit = "";

  private String mail_domain = "";
  private String mail_stripper = "";
  private String mail_format = "";

  private String smtp_host = "";
  private int smtp_port = 0;

  private int defaults_dailyLimt = 0;
  private int default_annotation_size = 0;


  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!isInitialized()) {
      try {
        store_dir = getConfig().getString("store.dir");

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

        admin_host = getConfig().getString("admin.host");
        admin_port = getConfig().getInt("admin.port");
        admin_allow_gsm_2_email_without_profile = getConfig().getBool("admin.allowGsm2EmlWithoutProfile");
        admin_allow_email_2_gsm_without_profile = getConfig().getBool("admin.allowEml2GsmWithoutProfile");
        admin_default_limit = getConfig().getString("admin.defaultLimit");

        mail_domain = getConfig().getString("mail.domain");
        mail_stripper = getConfig().getString("mail.stripper");
        mail_format = getConfig().getString("mail.format");

        smtp_host = getConfig().getString("smtp.host");
        smtp_port = getConfig().getInt("smtp.port");

        defaults_dailyLimt = getConfig().getInt("defaults.dailyLimit");
        default_annotation_size = getConfig().getInt("defaults.annotationSize");
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

    if (mbDone != null)
      return done();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int done()
  {
    getConfig().setString("store.dir", store_dir);

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

    getConfig().setString("admin.host", admin_host);
    getConfig().setInt("admin.port", admin_port);
    getConfig().setBool("admin.allowGsm2EmlWithoutProfile", admin_allow_gsm_2_email_without_profile);
    getConfig().setBool("admin.allowEml2GsmWithoutProfile", admin_allow_email_2_gsm_without_profile);
    getConfig().setString("admin.defaultLimit", admin_default_limit);

    getConfig().setString("mail.domain", mail_domain);
    getConfig().setString("mail.stripper", mail_stripper);
    getConfig().setString("mail.format", mail_format);

    getConfig().setString("smtp.host", smtp_host);
    getConfig().setInt("smtp.port", smtp_port);

    getConfig().setInt("defaults.dailyLimit", defaults_dailyLimt);
    getConfig().setInt("defaults.annotationSize", default_annotation_size);

//    try {
//      getConfig().save();
//    } catch (IOException e) {
//      logger.error("Cant save config", e);
//      e.printStackTrace();
//    } catch (Config.WrongParamTypeException e) {
//      logger.error("Cant save config", e);
//      e.printStackTrace();
//    }
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

  public String getStore_dir() {
    return store_dir;
  }

  public void setStore_dir(String store_dir) {
    this.store_dir = store_dir;
  }

  public String getAdmin_host() {
    return admin_host;
  }

  public void setAdmin_host(String admin_host) {
    this.admin_host = admin_host;
  }

  public String getAdmin_port() {
    return String.valueOf(admin_port);
  }

  public void setAdmin_port(String admin_port) {
    try {
      this.admin_port = Integer.decode(admin_port).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid admin.port parameter value: \"" + admin_port + '"', e);
    }
  }

  public String getAdmin_allow_gsm_2_email_without_profile() {
    return admin_allow_gsm_2_email_without_profile ? "checked" : "";
  }

  public void setAdmin_allow_gsm_2_email_without_profile(String admin_allow_gsm_2_email_without_profile) {
    this.admin_allow_gsm_2_email_without_profile = admin_allow_gsm_2_email_without_profile.equals("on");
  }

  public String getAdmin_allow_email_2_gsm_without_profile() {
    return admin_allow_email_2_gsm_without_profile ? "checked" : "";
  }

  public void setAdmin_allow_email_2_gsm_without_profile(String admin_allow_email_2_gsm_without_profile) {
    this.admin_allow_email_2_gsm_without_profile = admin_allow_email_2_gsm_without_profile.equals("on");
  }

  public String getAdmin_default_limit() {
    return admin_default_limit;
  }

  public void setAdmin_default_limit(String admin_default_limit) {
    this.admin_default_limit = admin_default_limit;
  }

  public String getMail_stripper() {
    return mail_stripper;
  }

  public void setMail_stripper(String mail_stripper) {
    this.mail_stripper = mail_stripper;
  }

  public String getMail_format() {
    return mail_format;
  }

  public void setMail_format(String mail_format) {
    this.mail_format = mail_format;
  }

  public String getDefault_annotation_size() {
    return String.valueOf(default_annotation_size);
  }

  public void setDefault_annotation_size(String default_annotation_size) {
    try {
      this.default_annotation_size = Integer.decode(default_annotation_size).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid default.annotation parameter value: \"" + default_annotation_size + '"', e);
    }
  }
}
