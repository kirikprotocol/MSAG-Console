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
  private String store_dir = "";
  private String store_queueDir = "store/eueue";

  private String smpp_host = "";
  private int smpp_port = 0;
  private int smpp_timeout = 0;
  private String smpp_systemId = "";
  private String smpp_password = "";
  private String smpp_sourceAddress = "";
  private String smpp_serviceType = "";
  private int smpp_protocolId = 0;
  private int smpp_retryTime = 0;
  private boolean smpp_partitionSms = false;
  private int smpp_partsSendSpeedPerHour = 60;
  private int smpp_maxUdhParts = 3;

  private boolean stat_enabled = false;
  private String stat_storeLocation = "store/stat";
  private int stat_flushPeriodInSec = 300;

  private String listener_host = "";
  private int listener_port = 0;

  private String admin_host = "";
  private int admin_port = 0;
  private boolean admin_allow_gsm_2_email_without_profile = false;
  private boolean admin_allow_email_2_gsm_without_profile = false;
  private boolean sendSuccessAnswer = false;
  private String admin_default_limit = "";
  private String admin_regionsconfig = "";
  private String admin_routesconfig = "";
  private String admin_helpdeskAddress = "";

  private String mail_domain = "";
  private String mail_stripper = "";
  private String mail_format = "";
  private String mail_user_name_transform_regexp;
  private String mail_user_name_transform_result;

  private String smtp_host = "";
  private int smtp_port = 0;

  private int defaults_dailyLimt = 0;
  private int default_annotation_size = 0;

  private String answers_alias = "";
  private String answers_aliasfailed = "";
  private String answers_aliasbusy = "";
  private String answers_noalias = "";
  private String answers_forward = "";
  private String answers_forwardfailed = "";
  private String answers_forwardoff = "";
  private String answers_realname = "";
  private String answers_numberon = "";
  private String answers_numberoff = "";
  private String answers_numberfailed = "";
  private String answers_systemerror = "";
  private String answers_unknowncommand = "";
  private String answers_messagesent = "";
  private String answers_messagefailedlimit = "";
  private String answers_messagefailednoprofile = "";
  private String answers_messagefailedsendmail = "";
  private String answers_messagefailedsystem = "";


  private String mbDone = null;
  private String mbCancel = null;

  private String getStringParameter(String name) {
    try {
      return getConfig().getString(name);
    } catch (Exception e) {
      logger.warn(e);
      return "";
    }
  }

  private int getIntParameter(String name) {
    try {
      return getConfig().getInt(name);
    } catch (Exception e) {
      logger.warn(e);
      return 0;
    }
  }

  private boolean getBoolParameter(String name) {
    try {
      return getConfig().getBool(name);
    } catch (Exception e) {
      logger.warn(e);
      return false;
    }
  }
  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!isInitialized()) {
      try {
        store_dir = getStringParameter("store.dir");
        store_queueDir = getStringParameter("store.queueDir");

        smpp_host = getStringParameter("smpp.host");
        smpp_port = getIntParameter("smpp.port");
        smpp_timeout = getIntParameter("smpp.timeout");
        smpp_systemId = getStringParameter("smpp.systemId");
        smpp_password = getStringParameter("smpp.password");
        smpp_sourceAddress = getStringParameter("smpp.sourceAddress");
        smpp_serviceType = getStringParameter("smpp.serviceType");
        smpp_protocolId = getIntParameter("smpp.protocolId");
        smpp_retryTime = getIntParameter("smpp.retryTime");
        smpp_partitionSms = getBoolParameter("smpp.partitionSms");
        smpp_partsSendSpeedPerHour = getIntParameter("smpp.partsSendSpeedPerHour");
        smpp_maxUdhParts = getIntParameter("smpp.maxUdhParts");

        if (getConfig().containsSection("stat") && getConfig().containsParameter("stat.storeLocation")) {
          stat_storeLocation = getStringParameter("stat.storeLocation");
          stat_flushPeriodInSec = getIntParameter("stat.flushPeriodInSec");
          stat_enabled = true;
        } else {
          stat_enabled = false;
        }

        listener_host = getStringParameter("listener.host");
        listener_port = getIntParameter("listener.port");

        admin_host = getStringParameter("admin.host");
        admin_port = getIntParameter("admin.port");
        admin_allow_gsm_2_email_without_profile = getBoolParameter("admin.allowGsm2EmlWithoutProfile");
        admin_allow_email_2_gsm_without_profile = getBoolParameter("admin.allowEml2GsmWithoutProfile");
        sendSuccessAnswer = getBoolParameter("answers.sendSuccessAnswer");
        admin_default_limit = getStringParameter("admin.defaultLimit");
        admin_regionsconfig = getStringParameter("admin.regionsconfig");
        admin_routesconfig = getStringParameter("admin.routesconfig");
        admin_helpdeskAddress = getStringParameter("admin.helpdeskAddress");

        mail_domain = getStringParameter("mail.domain");
        mail_stripper = getStringParameter("mail.stripper");
        mail_format = getStringParameter("mail.format");
        mail_user_name_transform_regexp = getStringParameter("mail.userNameTransformRegexp");
        mail_user_name_transform_result = getStringParameter("mail.userNameTransformResult");

        smtp_host = getStringParameter("smtp.host");
        smtp_port = getIntParameter("smtp.port");

        defaults_dailyLimt = getIntParameter("defaults.dailyLimit");
        default_annotation_size = getIntParameter("defaults.annotationSize");

        answers_alias = getStringParameter("answers.alias");
        answers_aliasfailed = getStringParameter("answers.aliasfailed");
        answers_aliasbusy = getStringParameter("answers.aliasbusy");
        answers_noalias = getStringParameter("answers.noalias");
        answers_forward = getStringParameter("answers.forward");
        answers_forwardfailed = getStringParameter("answers.forwardfailed");
        answers_forwardoff = getStringParameter("answers.forwardoff");
        answers_realname = getStringParameter("answers.realname");
        answers_numberon = getStringParameter("answers.numberon");
        answers_numberoff = getStringParameter("answers.numberoff");
        answers_numberfailed = getStringParameter("answers.numberfailed");
        answers_systemerror = getStringParameter("answers.systemerror");
        answers_unknowncommand = getStringParameter("answers.unknowncommand");
        answers_messagesent = getStringParameter("answers.messagesent");
        answers_messagefailedlimit = getStringParameter("answers.messagefailedlimit");
        answers_messagefailednoprofile = getStringParameter("answers.messagefailednoprofile");
        answers_messagefailedsendmail = getStringParameter("answers.messagefailedsendmail");
        answers_messagefailedsystem = getStringParameter("answers.messagefailedsystem");

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
    getConfig().setString("store.queueDir", store_queueDir);

    getConfig().setString("smpp.host", smpp_host);
    getConfig().setInt("smpp.port", smpp_port);
    getConfig().setInt("smpp.timeout", smpp_timeout);
    getConfig().setString("smpp.systemId", smpp_systemId);
    getConfig().setString("smpp.password", smpp_password);
    getConfig().setString("smpp.sourceAddress", smpp_sourceAddress);
    getConfig().setString("smpp.serviceType", smpp_serviceType);
    getConfig().setInt("smpp.protocolId", smpp_protocolId);
    getConfig().setInt("smpp.retryTime", smpp_retryTime);
    getConfig().setBool("smpp.partitionSms", smpp_partitionSms);
    getConfig().setInt("smpp.partsSendSpeedPerHour", smpp_partsSendSpeedPerHour);
    getConfig().setInt("smpp.maxUdhParts", smpp_maxUdhParts);

    if (stat_enabled) {
      getConfig().setString("stat.storeLocation", stat_storeLocation);
      getConfig().setInt("stat.flushPeriodInSec", stat_flushPeriodInSec);
    } else if (getConfig().containsSection("stat")) {
      getConfig().removeSection("stat");
    }

    getConfig().setString("listener.host", listener_host);
    getConfig().setInt("listener.port", listener_port);

    getConfig().setString("admin.host", admin_host);
    getConfig().setInt("admin.port", admin_port);
    getConfig().setBool("admin.allowGsm2EmlWithoutProfile", admin_allow_gsm_2_email_without_profile);
    getConfig().setBool("admin.allowEml2GsmWithoutProfile", admin_allow_email_2_gsm_without_profile);
    getConfig().setBool("answers.sendSuccessAnswer", sendSuccessAnswer);
    getConfig().setString("admin.defaultLimit", admin_default_limit);
    getConfig().setString("admin.regionsconfig", admin_regionsconfig);
    getConfig().setString("admin.routesconfig", admin_routesconfig);
    getConfig().setString("admin.helpdeskAddress", admin_helpdeskAddress);

    getConfig().setString("mail.domain", mail_domain);
    getConfig().setString("mail.stripper", mail_stripper);
    getConfig().setString("mail.format", mail_format);
    getConfig().setString("mail.userNameTransformRegexp", mail_user_name_transform_regexp);
    getConfig().setString("mail.userNameTransformResult", mail_user_name_transform_result);

    getConfig().setString("smtp.host", smtp_host);
    getConfig().setInt("smtp.port", smtp_port);

    getConfig().setInt("defaults.dailyLimit", defaults_dailyLimt);
    getConfig().setInt("defaults.annotationSize", default_annotation_size);


    getConfig().setString("answers.alias", answers_alias);
    getConfig().setString("answers.aliasfailed", answers_aliasfailed);
    getConfig().setString("answers.aliasbusy", answers_aliasbusy);
    getConfig().setString("answers.noalias", answers_noalias);
    getConfig().setString("answers.forward", answers_forward);
    getConfig().setString("answers.forwardfailed", answers_forwardfailed);
    getConfig().setString("answers.forwardoff", answers_forwardoff);
    getConfig().setString("answers.realname", answers_realname);
    getConfig().setString("answers.numberon", answers_numberon);
    getConfig().setString("answers.numberoff", answers_numberoff);
    getConfig().setString("answers.numberfailed", answers_numberfailed);
    getConfig().setString("answers.systemerror", answers_systemerror);
    getConfig().setString("answers.unknowncommand", answers_unknowncommand);
    getConfig().setString("answers.messagesent", answers_messagesent);
    getConfig().setString("answers.messagefailedlimit", answers_messagefailedlimit);
    getConfig().setString("answers.messagefailednoprofile", answers_messagefailednoprofile);
    getConfig().setString("answers.messagefailedsendmail", answers_messagefailedsendmail);
    getConfig().setString("answers.messagefailedsystem", answers_messagefailedsystem);

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

  public boolean isAdmin_allow_gsm_2_email_without_profile() {
    return admin_allow_gsm_2_email_without_profile;
  }

  public void setAdmin_allow_gsm_2_email_without_profile(boolean admin_allow_gsm_2_email_without_profile) {
    this.admin_allow_gsm_2_email_without_profile = admin_allow_gsm_2_email_without_profile;
  }

  public boolean isAdmin_allow_email_2_gsm_without_profile() {
    return admin_allow_email_2_gsm_without_profile;
  }

  public void setAdmin_allow_email_2_gsm_without_profile(boolean admin_allow_email_2_gsm_without_profile) {
    this.admin_allow_email_2_gsm_without_profile = admin_allow_email_2_gsm_without_profile;
  }

  public boolean isSendSuccessAnswer() {
    return sendSuccessAnswer;
  }

  public void setSendSuccessAnswer(boolean sendSuccessAnswer) {
    this.sendSuccessAnswer = sendSuccessAnswer;
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

  public String getMail_user_name_transform_regexp() {
    return mail_user_name_transform_regexp;
  }

  public void setMail_user_name_transform_regexp(String mail_user_name_transform_regexp) {
    this.mail_user_name_transform_regexp = mail_user_name_transform_regexp;
  }

  public String getMail_user_name_transform_result() {
    return mail_user_name_transform_result;
  }

  public void setMail_user_name_transform_result(String mail_user_name_transform_result) {
    this.mail_user_name_transform_result = mail_user_name_transform_result;
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

  public String getAnswers_alias() {
    return answers_alias;
  }

  public void setAnswers_alias(String answers_alias) {
    this.answers_alias = answers_alias;
  }

  public String getAnswers_aliasfailed() {
    return answers_aliasfailed;
  }

  public void setAnswers_aliasfailed(String answers_aliasfailed) {
    this.answers_aliasfailed = answers_aliasfailed;
  }

  public String getAnswers_aliasbusy() {
    return answers_aliasbusy;
  }

  public void setAnswers_aliasbusy(String answers_aliasbusy) {
    this.answers_aliasbusy = answers_aliasbusy;
  }

  public String getAnswers_noalias() {
    return answers_noalias;
  }

  public void setAnswers_noalias(String answers_noalias) {
    this.answers_noalias = answers_noalias;
  }

  public String getAnswers_forward() {
    return answers_forward;
  }

  public void setAnswers_forward(String answers_forward) {
    this.answers_forward = answers_forward;
  }

  public String getAnswers_forwardfailed() {
    return answers_forwardfailed;
  }

  public void setAnswers_forwardfailed(String answers_forwardfailed) {
    this.answers_forwardfailed = answers_forwardfailed;
  }

  public String getAnswers_forwardoff() {
    return answers_forwardoff;
  }

  public void setAnswers_forwardoff(String answers_forwardoff) {
    this.answers_forwardoff = answers_forwardoff;
  }

  public String getAnswers_realname() {
    return answers_realname;
  }

  public void setAnswers_realname(String answers_realname) {
    this.answers_realname = answers_realname;
  }

  public String getAnswers_numberon() {
    return answers_numberon;
  }

  public void setAnswers_numberon(String answers_numberon) {
    this.answers_numberon = answers_numberon;
  }

  public String getAnswers_numberoff() {
    return answers_numberoff;
  }

  public void setAnswers_numberoff(String answers_numberoff) {
    this.answers_numberoff = answers_numberoff;
  }

  public String getAnswers_numberfailed() {
    return answers_numberfailed;
  }

  public void setAnswers_numberfailed(String answers_numberfailed) {
    this.answers_numberfailed = answers_numberfailed;
  }

  public String getAnswers_systemerror() {
    return answers_systemerror;
  }

  public void setAnswers_systemerror(String answers_systemerror) {
    this.answers_systemerror = answers_systemerror;
  }

  public String getAnswers_unknowncommand() {
    return answers_unknowncommand;
  }

  public void setAnswers_unknowncommand(String answers_unknowncommand) {
    this.answers_unknowncommand = answers_unknowncommand;
  }

  public String getAnswers_messagesent() {
    return answers_messagesent;
  }

  public void setAnswers_messagesent(String answers_messagesent) {
    this.answers_messagesent = answers_messagesent;
  }

  public String getAnswers_messagefailedlimit() {
    return answers_messagefailedlimit;
  }

  public void setAnswers_messagefailedlimit(String answers_messagefailedlimit) {
    this.answers_messagefailedlimit = answers_messagefailedlimit;
  }

  public String getAnswers_messagefailednoprofile() {
    return answers_messagefailednoprofile;
  }

  public void setAnswers_messagefailednoprofile(String answers_messagefailednoprofile) {
    this.answers_messagefailednoprofile = answers_messagefailednoprofile;
  }

  public String getAnswers_messagefailedsendmail() {
    return answers_messagefailedsendmail;
  }

  public void setAnswers_messagefailedsendmail(String answers_messagefailedsendmail) {
    this.answers_messagefailedsendmail = answers_messagefailedsendmail;
  }

  public String getAnswers_messagefailedsystem() {
    return answers_messagefailedsystem;
  }

  public void setAnswers_messagefailedsystem(String answers_messagefailedsystem) {
    this.answers_messagefailedsystem = answers_messagefailedsystem;
  }

  public String getSmpp_maxUdhParts() {
    return String.valueOf(smpp_maxUdhParts);
  }

  public void setSmpp_maxUdhParts(String smpp_maxUdhParts) {
    try {
      this.smpp_maxUdhParts = Integer.parseInt(smpp_maxUdhParts);
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.maxUdhParts: " + smpp_maxUdhParts);
    }
  }

  public boolean isSmpp_partitionSms() {
    return smpp_partitionSms;
  }

  public void setSmpp_partitionSms(boolean smpp_partitionSms) {
    this.smpp_partitionSms = smpp_partitionSms;
  }

  public String getSmpp_partsSendSpeedPerHour() {
    return String.valueOf(smpp_partsSendSpeedPerHour);
  }

  public void setSmpp_partsSendSpeedPerHour(String smpp_partsSendSpeedPerHour) {
    try {
      this.smpp_partsSendSpeedPerHour = Integer.parseInt(smpp_partsSendSpeedPerHour);
    } catch (NumberFormatException e) {
      logger.error("Invalid smpp.partsSendSpeedPerHour: " + smpp_partsSendSpeedPerHour);
    }
  }

  public String getStore_queueDir() {
    return store_queueDir;
  }

  public void setStore_queueDir(String store_queueDir) {
    this.store_queueDir = store_queueDir;
  }

  public String getStat_flushPeriodInSec() {
    return String.valueOf(stat_flushPeriodInSec);
  }

  public void setStat_flushPeriodInSec(String stat_flushPeriodInSec) {
    try {
      this.stat_flushPeriodInSec = Integer.parseInt(stat_flushPeriodInSec);
    } catch (NumberFormatException e) {
      logger.error("Invalid stat.flushPerionInSec: " + stat_flushPeriodInSec);
    }
  }

  public String getStat_storeLocation() {
    return stat_storeLocation;
  }

  public void setStat_storeLocation(String stat_storeLocation) {
    this.stat_storeLocation = stat_storeLocation;
  }

  public String getAdmin_helpdeskAddress() {
    return admin_helpdeskAddress;
  }

  public void setAdmin_helpdeskAddress(String admin_helpdeskAddress) {
    this.admin_helpdeskAddress = admin_helpdeskAddress;
  }

  public String getAdmin_regionsconfig() {
    return admin_regionsconfig;
  }

  public void setAdmin_regionsconfig(String admin_regionsconfig) {
    this.admin_regionsconfig = admin_regionsconfig;
  }

  public String getAdmin_routesconfig() {
    return admin_routesconfig;
  }

  public void setAdmin_routesconfig(String admin_routesconfig) {
    this.admin_routesconfig = admin_routesconfig;
  }

  public boolean isStat_enabled() {
    return stat_enabled;
  }

  public void setStat_enabled(boolean val) {
    stat_enabled = val;
  }
}
