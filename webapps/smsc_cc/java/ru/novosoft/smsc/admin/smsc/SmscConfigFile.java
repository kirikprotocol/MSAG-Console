package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.XmlConfigHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/**
 * Вспомогательный класс для работы (загрузки/сохранения) с config.xml СМС центра.
 * Те пареметры, которые не отображаются на веб-интерфейсе, должны проноситься без изменений между load и save !!!
 *
 * @author Artem Snopkov
 */
class SmscConfigFile {


  private CommonSettings commonSettings;

  private InstanceSettings[] instanceSettings;

  private final File smscConfigFile;
  private final File backupDir;
  private final FileSystem fileSystem;

  public SmscConfigFile(File configFile, File backupDir, FileSystem fileSystem) {
    this.smscConfigFile = configFile;
    this.backupDir = backupDir;
    this.fileSystem = fileSystem;
  }

  public File getSmscConfigFile() {
    return smscConfigFile;
  }

  public File getBackupDir() {
    return backupDir;
  }

  private XmlConfig loadConfig() throws XmlConfigException, AdminException {
    InputStream is = null;
    try {
      is = fileSystem.getInputStream(smscConfigFile);
      return new XmlConfig(is);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
  }

  void load() throws AdminException {
    try {
      XmlConfig config = loadConfig();

      commonSettings = new CommonSettings(config);
      commonSettings.load(config);

      int instancesCount = config.getSection("cluster").getInt("nodesCount");
      instanceSettings = new InstanceSettings[instancesCount];
      for (int i = 0; i < instancesCount; i++) {
        instanceSettings[i] = new InstanceSettings(i, config);
      }
    } catch (XmlConfigException e) {
      throw new SmscException("invalid_config_file_format", e);
    }
  }

  void save() throws AdminException {
    try {
      XmlConfig config = loadConfig();
      commonSettings.save(config);
      for (int i=0; i<instanceSettings.length; i++) {
        instanceSettings[i].save(i, config);
      }
      XmlConfigHelper.saveXmlConfig(config, smscConfigFile, backupDir, fileSystem);
    } catch (XmlConfigException e) {
      throw new SmscException("save_error", e);
    }
  }

  CommonSettings getCommonSettings() {
    return commonSettings;
  }

  public void setCommonSettings(CommonSettings commonSettings) {
    this.commonSettings = commonSettings;
  }

  int getSmscInstancesCount() {
    return instanceSettings.length;
  }

  InstanceSettings getInstanceSettings(int instanceNumber) {
    if (instanceNumber < 0 || instanceNumber > instanceSettings.length - 1) {
      throw new IllegalArgumentException("Illegal instance number: " + instanceNumber + ". Must be in [0," + (instanceSettings.length - 1) + "]");
    }
    return instanceSettings[instanceNumber];
  }

  InstanceSettings[] getAllInstanceSettings() {
    return instanceSettings;
  }

  public void setInstanceSettings(int instanceNumber, InstanceSettings instanceSettings) {    
    if (instanceNumber < 0 || instanceNumber > this.instanceSettings.length - 1) {
      throw new IllegalArgumentException("Illegal instance number: " + instanceNumber + ". Must be in [0," + (this.instanceSettings.length - 1) + "]");
    }

    this.instanceSettings[instanceNumber] = instanceSettings;
  }
}
