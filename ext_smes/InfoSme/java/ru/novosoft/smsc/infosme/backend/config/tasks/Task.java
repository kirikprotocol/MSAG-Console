package ru.novosoft.smsc.infosme.backend.config.tasks;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.Config.WrongParamTypeException;

import java.util.*;
import java.text.SimpleDateFormat;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;

import org.xml.sax.SAXException;
import org.apache.log4j.Category;
import ru.novosoft.smsc.util.hsModeSupport.FileOutputStreamMirror;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.10.2003
 * Time: 17:10:35
 */
public class Task extends Observable
{
  private static final Category logger = Category.getInstance(Task.class);

  public final static String[] WEEK_DAYS = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
  private final static String DEFAULT_ACTIVE_WEEK_DAYS = "Mon,Tue,Wed,Thu,Fri";
  private final static SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
  private static final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  public static final String INFOSME_EXT_PROVIDER = "EXTERNAL";
  public static final String CONFIG_FILE_NAME = "config.xml";

  public static final int DELIVERY_MODE_SMS = 0;
  public static final int DELIVERY_MODE_USSD_PUSH = 1;
  public static final int DELIVERY_MODE_USSD_PUSH_VLR = 2;

  private boolean modified;

  // General
  private String id;
  private String owner = "";
  private String name = "";
  private String address = "";
  private String provider = "";
  private boolean enabled = false;
  private int priority = 0;
  private boolean replaceMessage = false;
  private String svcType = "";
  private boolean useDataSm = false;
  private Integer deliveryMode = new Integer(DELIVERY_MODE_SMS);

  // Retry on fail
  private boolean retryOnFail = false;
  private String retryPolicy = "";
//  private String retryTime = "";


  // Time arguments
  private Date endDate = null;
  private Date startDate = null;
  private Integer validityPeriod = null;
  private Date validityDate = null;
  private Date activePeriodStart = null;
  private Date activePeriodEnd = null;
  private Collection activeWeekDaysSet = new HashSet(WEEK_DAYS.length);

  // Other
  private String query = "";
  private String template = "";
  private String text = "";
  private int dsTimeout = 0;
  private int messagesCacheSize = 0;
  private int messagesCacheSleep = 0;
  private boolean transactionMode = false;
  private boolean keepHistory = false;
  // temporary switched to true
  private boolean saveFinalState = false;
  private boolean flash = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;
  private boolean trackIntegrity;
  private boolean delivery = false;

  // for new deliveries
  private int regionId;
  private File deliveriesFile;
  private boolean deliveriesFileContainsTexts;
  private int actualRecordsSize;
  private boolean messagesHaveLoaded = false;
  private String location;
  // for secret
  private boolean secret;
  private boolean secretFlash;
  private String secretMessage="";

  private final String storeLocation;
  private final Integer volumeSize;

  Task(String storeLocation, Integer volumeSize) {
    activeWeekDaysSet = new HashSet(WEEK_DAYS.length);
    Functions.addValuesToCollection(this.activeWeekDaysSet, DEFAULT_ACTIVE_WEEK_DAYS, ",", true);
    if (delivery)
      provider = Task.INFOSME_EXT_PROVIDER;
    this.modified = true;
    this.storeLocation = storeLocation;
    this.volumeSize = volumeSize;
  }

  public Task(Config config, String id, String storeLocation, Integer volumeSize) throws Config.WrongParamTypeException, Config.ParamNotFoundException,
      IOException, ParserConfigurationException, SAXException {
    this(storeLocation, volumeSize);
    setId(id);
    File configFile = new File(location, CONFIG_FILE_NAME);
    if (configFile.exists()) {
      loadConfig(new Config(configFile), "");
      logger.info("Task " + id + " loaded from separate config");
    } else {
      if(config == null) {
        throw new IllegalArgumentException("Config is null");
      }
      loadConfig(config, TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id) + '.');
      logger.info("Task " + id + " loaded from common config");
    }
  }

  void setId(String id) {
    this.id = id;

    if (volumeSize == null)
      location = storeLocation + File.separatorChar + id;
    else {
      String volumeNumberStr = String.valueOf(Long.parseLong(id) / volumeSize.intValue() * volumeSize.intValue());
      String chunkTemplate = "chunk0000000000";
      volumeNumberStr = chunkTemplate.substring(0, chunkTemplate.length() - volumeNumberStr.length()) + volumeNumberStr;
      location = storeLocation + File.separator + volumeNumberStr + File.separator + id;
    }

  }

  private void loadConfig(Config config, String prefix) throws Config.ParamNotFoundException, Config.WrongParamTypeException {
    name = config.getString(prefix + "name");
    address = (config.containsParameter(prefix + "address")) ? config.getString(prefix + "address") : "";
    owner = config.containsParameter(prefix + "owner") ? config.getString(prefix + "owner") : null;

    provider = config.getString(prefix + "dsId");

    priority = config.getInt(prefix + "priority");

    enabled = config.getBool(prefix + "enabled");
    endDate = getDateFromConfig(config, prefix + "endDate", df);
    startDate = getDateFromConfig(config, prefix + "startDate", df);
    activePeriodStart = getDateFromConfig(config, prefix + "activePeriodStart", tf);
    activePeriodEnd = getDateFromConfig(config, prefix + "activePeriodEnd", tf);
    String activeWeekDaysStr = config.containsParameter(prefix + "activeWeekDays") ? config.getString(prefix + "activeWeekDays") : DEFAULT_ACTIVE_WEEK_DAYS;
    activeWeekDaysSet = new HashSet(WEEK_DAYS.length);
    Functions.addValuesToCollection(this.activeWeekDaysSet, activeWeekDaysStr, ",", true);

    transactionMode = config.getBool(prefix + "transactionMode");
    validityPeriod = parseValidityPeriod(config, prefix + "validityPeriod");
    validityDate = getDateFromConfig(config, prefix + "validityDate", df);
    replaceMessage = config.getBool(prefix + "replaceMessage");
    svcType = config.getString(prefix + "svcType");
    retryOnFail = config.getBool(prefix + "retryOnFail");
    retryPolicy = config.getString(prefix + "retryPolicy");
    flash = config.containsParameter(prefix + "flash") && config.getBool(prefix + "flash");

    query = config.getString(prefix + "query");
    template = config.getString(prefix + "template");
    dsTimeout = config.getInt(prefix + "dsTimeout");
    messagesCacheSize = config.getInt(prefix + "messagesCacheSize");
    messagesCacheSleep = config.getInt(prefix + "messagesCacheSleep");
    uncommitedInGeneration = config.getInt(prefix + "uncommitedInGeneration");
    uncommitedInProcess = config.getInt(prefix + "uncommitedInProcess");
    trackIntegrity = config.getBool(prefix + "trackIntegrity");
    keepHistory = config.getBool(prefix + "keepHistory");
    if (config.containsParameter(prefix + "saveFinalState"))
      saveFinalState = config.getBool(prefix + "saveFinalState");
    messagesHaveLoaded = config.getBool(prefix + "messagesHaveLoaded");
    if (config.containsParameter(prefix + "useDataSm"))
      useDataSm = config.getBool(prefix + "useDataSm");

    delivery  = config.containsParameter(prefix + "delivery") && config.getBool(prefix + "delivery");
    if (delivery)
      provider = Task.INFOSME_EXT_PROVIDER;

    try {
      secret = config.getBool(prefix + "secret");
      secretFlash = config.getBool(prefix + "secretFlash");
      secretMessage = config.getString(prefix + "secretMessage");
    } catch (Config.ParamNotFoundException e) {
      secret = false;
      secretFlash = false;
      secretMessage = "";
    }

    Boolean useUssdPush = null;
    if (config.containsParameter(prefix + "useUssdPush"))
      useUssdPush = Boolean.valueOf(config.getBool(prefix + "useUssdPush"));

    if (config.containsParameter(prefix + "deliveryMode")) {
      String deliveryModeStr = config.getString("deliveryMode");
      if (deliveryModeStr.equalsIgnoreCase("sms"))
        deliveryMode = new Integer(DELIVERY_MODE_SMS);
      else if (deliveryModeStr.equalsIgnoreCase("ussd_push"))
        deliveryMode = new Integer(DELIVERY_MODE_USSD_PUSH);
      else
        deliveryMode = new Integer(DELIVERY_MODE_USSD_PUSH_VLR);
    }

    if (useUssdPush != null)
      deliveryMode = useUssdPush == Boolean.FALSE ? new Integer(DELIVERY_MODE_SMS) : new Integer(DELIVERY_MODE_USSD_PUSH);

    if ((deliveryMode != null && deliveryMode.intValue() != DELIVERY_MODE_SMS)) {
      // true
      useDataSm = false;
      transactionMode = true;
      flash = false;
    }

    this.modified = false;
  }

  private static Integer parseValidityPeriod(Config config, String paramName) {
    if(config.containsParameter(paramName)) {
      try {
        String str = config.getString(paramName);
        if(str.length() > 0) {
          str = str.substring(0, str.indexOf(":"));
          return new Integer(Integer.parseInt(str));
        }
      } catch (Exception e) {
        return null;
      }
    }
    return null;

  }

  private static Date getDateFromConfig(Config config, String paramName, SimpleDateFormat df) {
    try {
      return df.parse(config.getString(paramName));
    } catch (Throwable e) {
      return null;
    }
  }

  public static boolean existsConfigFile(String fileLocation) {
    return new File(fileLocation, CONFIG_FILE_NAME).exists();
  }

  private File createConfigFile() throws IOException {
    if(id == null) {
      throw new IllegalArgumentException("Id is not set!");
    }
    File configFile = new File(location, CONFIG_FILE_NAME);
    if (!configFile.createNewFile()) {
      return configFile;
    }
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configFile), Functions.getLocaleEncoding()));
      Functions.storeConfigHeader(out, "config", "configuration.dtd", Functions.getLocaleEncoding());
      Functions.storeConfigFooter(out, "config");
    } finally {
      if (out != null) out.close();
    }
    return configFile;
  }

  //void storeToConfig(Config config)
  void storeToConfig() throws IOException, SAXException, ParserConfigurationException, WrongParamTypeException
  {
    if(id == null) {
      throw new IllegalArgumentException("Id is not set");
    }
    //final String prefix = separateConfig ? "" : TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id) + '.';
    String prefix = "";
    File configDir = new File(location);
    if (!configDir.exists()) {
      configDir.mkdirs();
    }
    File configFile = this.createConfigFile();
    Config config = new Config(configFile);
    config.setString(prefix + "name", name);
    if (address != null && address.trim().length() > 0)
      config.setString(prefix + "address", address.trim());
    config.setString(prefix + "dsId", provider);
    config.setBool(prefix + "enabled", enabled);
    config.setBool(prefix + "delivery", delivery);
    config.setInt(prefix + "priority", priority);
    config.setBool(prefix + "retryOnFail", retryOnFail);
    config.setBool(prefix + "replaceMessage", replaceMessage);
    config.setString(prefix + "svcType", svcType);
    config.setString(prefix + "startDate", startDate == null ? "" : df.format(startDate));
    config.setString(prefix + "endDate", endDate == null ? "" : df.format(endDate));
    config.setString(prefix + "retryPolicy", retryPolicy);

    String validityPeriodStr = "";
    if (validityPeriod != null) {
      validityPeriodStr = validityPeriod + ":00:00";
      if (validityPeriod.intValue() < 10)
        validityPeriodStr = "0" + validityPeriodStr;
    }

    config.setString(prefix + "validityPeriod", validityPeriodStr);
    config.setString(prefix + "validityDate", validityDate == null ? "" : df.format(validityDate));
    config.setString(prefix + "activePeriodStart", activePeriodStart == null ? "" : tf.format(activePeriodStart));
    config.setString(prefix + "activePeriodEnd", activePeriodEnd == null ? "" : tf.format(activePeriodEnd));
    config.setString(prefix + "query", query);
    config.setString(prefix + "template", template);
    config.setInt(prefix + "dsTimeout", dsTimeout);
    config.setInt(prefix + "messagesCacheSize", messagesCacheSize);
    config.setInt(prefix + "messagesCacheSleep", messagesCacheSleep);
    config.setBool(prefix + "transactionMode", transactionMode);
    config.setInt(prefix + "uncommitedInGeneration", uncommitedInGeneration);
    config.setInt(prefix + "uncommitedInProcess", uncommitedInProcess);
    config.setBool(prefix + "trackIntegrity", trackIntegrity);
    config.setBool(prefix + "keepHistory", keepHistory);
    config.setBool(prefix + "saveFinalState", saveFinalState );
    config.setBool(prefix + "useDataSm", useDataSm );

    String deliveryModeStr;
    switch (deliveryMode.intValue()) {
      case DELIVERY_MODE_SMS: deliveryModeStr = "SMS"; break;
      case DELIVERY_MODE_USSD_PUSH: deliveryModeStr = "USSD_PUSH"; break;
      default: deliveryModeStr = "USSD_PUSH_VLR";
    }
    config.setString(prefix + "deliveryMode", deliveryModeStr);

    config.setBool(prefix + "flash", flash);
    config.setString(prefix + "activeWeekDays", Functions.collectionToString(activeWeekDaysSet, ","));
    config.setBool(prefix + "messagesHaveLoaded", messagesHaveLoaded);
    config.setBool(prefix + "secret", secret);
    config.setBool(prefix + "secretFlash", secretFlash);
    config.setString(prefix + "secretMessage", secretMessage);
    config.setString(prefix + "owner", owner);
    config.save();
  }

  public boolean isUserSeparateConfig() {
    File configFile = new File(location, CONFIG_FILE_NAME);
    return configFile.exists();
  }

  public void remove(Config config) {
    if(id == null) {
      throw new IllegalArgumentException("Id is not set");
    }
    File configFile = new File(location, CONFIG_FILE_NAME);
    if (configFile.exists()) {
      configFile.renameTo(new File(location, CONFIG_FILE_NAME + ".bak"));
    } else {
      config.removeSection(TaskDataSource.TASKS_PREFIX  + '.' + id);
    }
  }


  private static final String archiveDirPattern = "yyyy"+File.separatorChar+"MM"+File.separatorChar+"dd"+File.separatorChar;

  public void archivate(Config config, String archiveDir) throws IOException{
    if(id == null) {
      throw new IllegalArgumentException("Id is not set");
    }
    File configFile = new File(location, CONFIG_FILE_NAME);
    if (!configFile.exists()) {
      config.removeSection(TaskDataSource.TASKS_PREFIX  + '.' + id);
    }

    File taskDir = new File(location);
    File toDir  = new File(new File(archiveDir),
        new StringBuffer().append(
            new SimpleDateFormat(archiveDirPattern).format(startDate)
        ).append(
            id
        ).toString());
    if(!toDir.exists() && !toDir.mkdirs()) {
      String error = new StringBuffer().append("Can't create directory to archivate task: ").append(toDir.getAbsolutePath()).toString();
      logger.error(error);
      throw new IOException(error);
    }
    copyDirs(taskDir, toDir);
    removeDir(taskDir);
    if(logger.isDebugEnabled()) {
      logger.debug(new StringBuffer(100).append("Task is replaced to archive: ").
          append(id).append(" (").append(name).append(')').toString());
    }

  }

  private static void copyDirs(File source, File destination) throws IOException {
    File[] children = source.listFiles();
    for(int i=0; i<children.length; i++) {
      File f = children[i];
      if(f.isFile()) {
        System.out.println("Copy file "+f.getAbsolutePath()+" to "+new File(destination, f.getName()));
        copyFile(f, new File(destination, f.getName()));
      }
      if(f.isDirectory()) {
        File newDir = new File(destination, f.getName());
        if(!newDir.exists() && !newDir.mkdirs()) {
          String error = new StringBuffer().append("Can't create directory: ").
              append(newDir.getAbsolutePath()).toString();
          logger.error(error);
          throw new IOException(error);
        }
        copyDirs(f, newDir);
      }
    }
  }

  private static void removeDir(File dir) throws IOException{
    File[] children = dir.listFiles();
    for(int i=0; i<children.length; i++) {
      File f = children[i];
      if(f.isFile()) {
        if(!f.delete()) {
          logger.error("Can't remove: "+f.getAbsolutePath());
          throw new IOException("Can't remove: "+f.getAbsolutePath());
        }
      }else {
        removeDir(f);
      }
    }
    if(!dir.delete()) {
      logger.error("Can't remove: "+dir.getAbsolutePath());
      throw new IOException("Can't remove: "+dir.getAbsolutePath());
    }
  }

  private static void copyFile(File source, File destination) throws IOException{
    BufferedInputStream is = null;
    BufferedOutputStream os = null;
    try{
      is = new BufferedInputStream(new FileInputStream(source));
      os = new BufferedOutputStream(new FileOutputStreamMirror(destination));
      byte[] buf = new byte[1024];
      int len;
      while ((len = is.read(buf)) > 0){
        os.write(buf, 0, len);
      }
    }finally {
      if(is != null) {
        try{
        is.close();
        }catch (IOException e){}
      }
      if(os != null) {
        try{
        os.close();
        }catch (IOException e){}
      }
    }

  }

  public void change(Config config) throws IOException, SAXException, ParserConfigurationException, WrongParamTypeException {
    if(id == null) {
      throw new IllegalArgumentException("Id is not set");
    }
    File locationFile = new File(location);
    if (!locationFile.exists())
      locationFile.mkdirs();
    File configFile = new File(location, CONFIG_FILE_NAME);
    if (configFile.exists()) {
      this.storeToConfig();
    } else {
      config.removeSection(TaskDataSource.TASKS_PREFIX  + '.' + id);
      this.storeToConfig();
    }
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    final Task task = (Task) o;

    if (actualRecordsSize != task.actualRecordsSize) return false;
    if (deliveriesFileContainsTexts != task.deliveriesFileContainsTexts) return false;
    if (delivery != task.delivery) return false;
    if (dsTimeout != task.dsTimeout) return false;
    if (enabled != task.enabled) return false;
    if (flash != task.flash) return false;
    if (keepHistory != task.keepHistory) return false;
    if (messagesCacheSize != task.messagesCacheSize) return false;
    if (messagesCacheSleep != task.messagesCacheSleep) return false;
    if (messagesHaveLoaded != task.messagesHaveLoaded) return false;
    if (modified != task.modified) return false;
    if (priority != task.priority) return false;
    if (regionId != task.regionId) return false;
    if (replaceMessage != task.replaceMessage) return false;
    if (retryOnFail != task.retryOnFail) return false;
    if (saveFinalState != task.saveFinalState) return false;
    if (secret != task.secret) return false;
    if (secretFlash != task.secretFlash) return false;
    if (trackIntegrity != task.trackIntegrity) return false;
    if (transactionMode != task.transactionMode) return false;
    if (uncommitedInGeneration != task.uncommitedInGeneration) return false;
    if (uncommitedInProcess != task.uncommitedInProcess) return false;
    if (useDataSm != task.useDataSm) return false;
    if (activePeriodEnd != null ? !activePeriodEnd.equals(task.activePeriodEnd) : task.activePeriodEnd != null)
      return false;
    if (activePeriodStart != null ? !activePeriodStart.equals(task.activePeriodStart) : task.activePeriodStart != null)
      return false;
    if (activeWeekDaysSet != null ? !activeWeekDaysSet.equals(task.activeWeekDaysSet) : task.activeWeekDaysSet != null)
      return false;
    if (address != null ? !address.equals(task.address) : task.address != null) return false;
    if (deliveriesFile != null ? !deliveriesFile.equals(task.deliveriesFile) : task.deliveriesFile != null) return false;
    if (endDate != null ? !endDate.equals(task.endDate) : task.endDate != null) return false;
    if (id != null ? !id.equals(task.id) : task.id != null) return false;
    if (location != null ? !location.equals(task.location) : task.location != null) return false;
    if (name != null ? !name.equals(task.name) : task.name != null) return false;
    if (owner != null ? !owner.equals(task.owner) : task.owner != null) return false;
    if (provider != null ? !provider.equals(task.provider) : task.provider != null) return false;
    if (query != null ? !query.equals(task.query) : task.query != null) return false;
    if (retryPolicy != null ? !retryPolicy.equals(task.retryPolicy) : task.retryPolicy != null) return false;
    if (secretMessage != null ? !secretMessage.equals(task.secretMessage) : task.secretMessage != null) return false;
    if (startDate != null ? !startDate.equals(task.startDate) : task.startDate != null) return false;
    if (storeLocation != null ? !storeLocation.equals(task.storeLocation) : task.storeLocation != null) return false;
    if (svcType != null ? !svcType.equals(task.svcType) : task.svcType != null) return false;
    if (template != null ? !template.equals(task.template) : task.template != null) return false;
    if (text != null ? !text.equals(task.text) : task.text != null) return false;
    if (validityDate != null ? !validityDate.equals(task.validityDate) : task.validityDate != null) return false;
    if (validityPeriod != null ? !validityPeriod.equals(task.validityPeriod) : task.validityPeriod != null) return false;
    if ((deliveryMode == null) != (task.deliveryMode == null) || (deliveryMode != null && task.deliveryMode != null && !deliveryMode.equals(deliveryMode))) return false;

    return true;
  }

  public String toString() {
    final StringBuffer sb = new StringBuffer();
    sb.append("Task");
    sb.append("{activePeriodEnd='").append(activePeriodEnd).append('\'');
    sb.append(", modified=").append(modified);
    sb.append(", id='").append(id).append('\'');
    sb.append(", owner='").append(owner).append('\'');
    sb.append(", name='").append(name).append('\'');
    sb.append(", address='").append(address).append('\'');
    sb.append(", provider='").append(provider).append('\'');
    sb.append(", enabled=").append(enabled);
    sb.append(", priority=").append(priority);
    sb.append(", retryOnFail=").append(retryOnFail);
    sb.append(", replaceMessage=").append(replaceMessage);
    sb.append(", svcType='").append(svcType).append('\'');
    sb.append(", endDate='").append(endDate).append('\'');
    sb.append(", startDate='").append(startDate).append('\'');
    sb.append(", validityPeriod='").append(validityPeriod).append('\'');
    sb.append(", validityDate='").append(validityDate).append('\'');
    sb.append(", activePeriodStart='").append(activePeriodStart).append('\'');
    sb.append(", activeWeekDays=").append(activeWeekDaysSet);
    sb.append(", activeWeekDaysSet=").append(activeWeekDaysSet);
    sb.append(", query='").append(query).append('\'');
    sb.append(", template='").append(template).append('\'');
    sb.append(", text='").append(text).append('\'');
    sb.append(", dsTimeout=").append(dsTimeout);
    sb.append(", retryPolicy='").append(retryPolicy).append('\'');
    sb.append(", messagesCacheSize=").append(messagesCacheSize);
    sb.append(", messagesCacheSleep=").append(messagesCacheSleep);
    sb.append(", transactionMode=").append(transactionMode);
    sb.append(", keepHistory=").append(keepHistory);
    sb.append(", saveFinalState=").append(saveFinalState);
    sb.append(", useDataSm=").append(useDataSm);
    sb.append(", flash=").append(flash);
    sb.append(", uncommitedInGeneration=").append(uncommitedInGeneration);
    sb.append(", uncommitedInProcess=").append(uncommitedInProcess);
    sb.append(", trackIntegrity=").append(trackIntegrity);
    sb.append(", delivery=").append(delivery);
    sb.append(", regionId='").append(regionId).append('\'');
    sb.append(", actualRecordsSize=").append(actualRecordsSize);
    sb.append(", messagesHaveLoaded=").append(messagesHaveLoaded);
    sb.append(", secret=").append(secret);
    sb.append(", secretFlash=").append(secretFlash);
    sb.append(", secretMessage='").append(secretMessage).append('\'');
    sb.append(", deliveryMode='").append(deliveryMode).append('\'');
    sb.append('}');
    return sb.toString();
  }

  /* ************************************* properties ************************************* */

  public String getId() {
    return id;
  }

  public String getName() {
    return name;
  }
  public void setName(String name) {
    this.name = name;
    modified = true;
  }

  public String getAddress() {
    return address;
  }
  public void setAddress(String address) {
    this.address = address;
    modified = true;
  }

  public String getProvider() {
    return provider;
  }

  public void setProvider(String provider) {
    this.provider = provider;
    modified = true;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
    modified = true;
  }

  public boolean isDelivery() {
    return delivery;
  }

  public void setDelivery(boolean delivery) {
    this.delivery = delivery;
    modified = true;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
    modified = true;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
    modified = true;
  }

  public boolean isReplaceMessage() {
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
    modified = true;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
    modified = true;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
    modified = true;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
    modified = true;
  }

  public Integer getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Integer validityPeriod) {
    this.validityPeriod = validityPeriod;
    modified = true;
  }

  public Date getValidityDate() {
    return validityDate;
  }

  public void setValidityDate(Date validityDate) {
    this.validityDate = validityDate;
    modified = true;
  }

  public Date getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Date activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
    modified = true;
  }

  public Date getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Date activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
    modified = true;
  }

  public String getQuery() {
    return query;
  }

  public void setQuery(String query) {
    this.query = query;
    modified = true;
  }

  public String getTemplate() {
    return template;
  }

  public void setTemplate(String template) {
    this.template = template;
    modified = true;
  }

  public String getText() {
    return text;
  }
  public void setText(String text) {
    this.text = text;
    modified = true;
  }

  public int getDsTimeout() {
    return dsTimeout;
  }
  public void setDsTimeout(int dsTimeout) {
    this.dsTimeout = dsTimeout;
    modified = true;
  }

  public int getMessagesCacheSize() {
    return messagesCacheSize;
  }

  public void setMessagesCacheSize(int messagesCacheSize) {
    this.messagesCacheSize = messagesCacheSize;
    modified = true;
  }

  public int getMessagesCacheSleep() {
    return messagesCacheSleep;
  }

  public void setMessagesCacheSleep(int messagesCacheSleep) {
    this.messagesCacheSleep = messagesCacheSleep;
    modified = true;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
    modified = true;
  }

  public int getUncommitedInGeneration() {
    return uncommitedInGeneration;
  }

  public void setUncommitedInGeneration(int uncommitedInGeneration) {
    this.uncommitedInGeneration = uncommitedInGeneration;
    modified = true;
  }

  public int getUncommitedInProcess() {
    return uncommitedInProcess;
  }

  public void setUncommitedInProcess(int uncommitedInProcess) {
    this.uncommitedInProcess = uncommitedInProcess;
    modified = true;
  }

  public boolean isTrackIntegrity() {
    return trackIntegrity;
  }

  public void setTrackIntegrity(boolean trackIntegrity) {
    this.trackIntegrity = trackIntegrity;
    modified = true;
  }

  public boolean isKeepHistory() {
    return keepHistory;
  }

  public void setKeepHistory(boolean keepHistory) {
    this.keepHistory = keepHistory;
    modified = true;
  }

  public boolean isSaveFinalState() {
    return saveFinalState;
  }

  public void setSaveFinalState(boolean saveFinalState) {
    this.saveFinalState = saveFinalState;
    modified = true;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    if ( deliveryMode != null && deliveryMode.intValue() == DELIVERY_MODE_SMS ) {
      this.useDataSm = useDataSm;
      modified = true;
    }
  }

  public int getDeliveryMode() {
    return deliveryMode.intValue();
  }

  public void setDeliveryMode(int deliveryMode) {
    if (deliveryMode < 0 || deliveryMode > DELIVERY_MODE_USSD_PUSH_VLR)
      throw new IllegalArgumentException();
    this.deliveryMode = new Integer(deliveryMode);

    if (deliveryMode != DELIVERY_MODE_SMS) {
      this.useDataSm = false;
      this.transactionMode = true;
      this.flash = false;
    }        
    modified = true;
  }

  public Collection getActiveWeekDays() {
    return activeWeekDaysSet;
  }

  public void setActiveWeekDays(Collection activeWeekDays) {
    this.activeWeekDaysSet = new HashSet(activeWeekDays);
    modified = true;
  }

  public boolean isWeekDayActive(String weekday) {
    return activeWeekDaysSet.contains(weekday);
  }

  public int getRegionId() {
    return regionId;
  }

  public void setRegionId(int id) {
    this.regionId = id;
    modified = true;
  }

  public int getActualRecordsSize() {
    return actualRecordsSize;
  }

  public void setActualRecordsSize(int actualRecordsSize) {
    this.actualRecordsSize = actualRecordsSize;
    modified = true;
  }

  public Collection getActiveWeekDaysSet() {
    return activeWeekDaysSet;
  }

  public void setActiveWeekDaysSet(Collection activeWeekDaysSet) {
    this.activeWeekDaysSet = activeWeekDaysSet;
    modified = true;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    if ( deliveryMode.intValue() == DELIVERY_MODE_SMS) {
      this.flash = flash;
      modified = true;
    }
  }

  public boolean isMessagesHaveLoaded() {
    return messagesHaveLoaded;
  }

  public void setMessagesHaveLoaded(boolean messagesHaveLoaded) {
    this.messagesHaveLoaded = messagesHaveLoaded;
    modified = true;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
    modified = true;
  }

  public boolean isSecret() {
    return secret;
  }

  public void setSecret(boolean secret) {
    this.secret = secret;
    modified = true;
  }

  public boolean isSecretFlash() {
    return secretFlash;
  }

  public void setSecretFlash(boolean secretFlash) {
    this.secretFlash = secretFlash;
    modified = true;
  }

  public String getSecretMessage() {
    return secretMessage;
  }

  public void setSecretMessage(String secretMessage) {
    this.secretMessage = secretMessage;
    modified = true;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner == null ? "" : owner;
    modified = true;
  }

  public File getDeliveriesFile() {
    return deliveriesFile;
  }

  public void setDeliveriesFile(File deliveriesFile) {
    this.deliveriesFile = deliveriesFile;
  }

  public boolean isDeliveriesFileContainsTexts() {
    return deliveriesFileContainsTexts;
  }

  public void setDeliveriesFileContainsTexts(boolean deliveriesFileContainsTexts) {
    this.deliveriesFileContainsTexts = deliveriesFileContainsTexts;
  }

  public void setModified(boolean modified) {
    this.modified = modified;
  }

  public boolean isModified() {
    return modified;
  }

  public String getLocation() {
    return location;
  }

  public Task cloneTask() {
    Task t = new Task(this.storeLocation, this.volumeSize);
    if(this.id != null) {
      t.setId(this.id);
    }

    t.modified = this.modified;

    // General
    t.owner = this.owner;
    t.name = this.name;
    t.address = this.address;
    t.provider = this.provider;
    t.enabled = this.enabled;
    t.priority = this.priority;
    t.replaceMessage = this.replaceMessage;
    t.svcType = this.svcType;
    t.useDataSm = this.useDataSm;
    t.deliveryMode = this.deliveryMode;

    // Retry on fail
    t.retryOnFail = this.retryOnFail;
    t.retryPolicy = this.retryPolicy;
//   t.retryTime = this."";


    // Time arguments
    t.endDate = this.endDate == null ? null : new Date(this.endDate.getTime());
    t.startDate = this.startDate == null ? null : new Date(this.startDate.getTime());
    t.validityPeriod = this.validityPeriod;
    t.validityDate= this.validityDate == null ? null : new Date(this.validityDate.getTime());
    t.activePeriodStart = this.activePeriodStart == null ? null : new Date(this.activePeriodStart.getTime());
    t.activePeriodEnd = this.activePeriodEnd == null ? null : new Date(this.activePeriodEnd.getTime());
    t.activeWeekDaysSet = this.activeWeekDaysSet == null ? null : new HashSet(this.activeWeekDaysSet);

    // Other
    t.query = this.query;
    t.template = this.template;
    t.text = this.text;
    t.dsTimeout = this.dsTimeout;
    t.messagesCacheSize = this.messagesCacheSize;
    t.messagesCacheSleep = this.messagesCacheSleep;
    t.transactionMode = this.transactionMode;
    t.keepHistory = this.keepHistory;
    // temporary switched to true
    t.saveFinalState = this.saveFinalState;
    t.flash = this.flash;
    t.uncommitedInGeneration = this.uncommitedInGeneration;
    t.uncommitedInProcess = this.uncommitedInProcess;
    t.trackIntegrity = this.trackIntegrity;
    t.delivery = this.delivery;

    // for new deliveries
    t.regionId = this.regionId;
    t.deliveriesFile = this.deliveriesFile == null ? null : new File(this.deliveriesFile.getAbsolutePath());
    t.deliveriesFileContainsTexts = this.deliveriesFileContainsTexts;
    t.actualRecordsSize = this.actualRecordsSize;
    t.messagesHaveLoaded = this.messagesHaveLoaded;
    t.location = this.location;
    // for secret
    t.secret = this.secret;
    t.secretFlash = this.secretFlash;
    t.secretMessage = this.secretMessage;

    return  t;

  }
}
