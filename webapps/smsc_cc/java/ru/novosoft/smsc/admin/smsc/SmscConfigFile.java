package ru.novosoft.smsc.admin.smsc;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.XmlConfigHelper;
import ru.novosoft.smsc.util.config.ConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Вспомогательный класс для работы (загрузки/сохранения) с config.xml СМС центра.
 * Те пареметры, которые не отображаются на веб-интерфейсе, должны проноситься без изменений между load и save !!!
 *
 * @author Artem Snopkov
 */
class SmscConfigFile {

  private static final Logger logger = Logger.getLogger(SmscConfigFile.class);

  private CommonSettings commonSettings;

  private InstanceSettings[] instanceSettings;

  private final File smscConfigFile;
  private final File backupDir;
  private final FileSystem fileSystem;

  public SmscConfigFile(File smscBaseDir, FileSystem fileSystem) {
    this.smscConfigFile = new File(smscBaseDir, "conf" + File.separator + "config.xml");
    this.backupDir = new File(smscBaseDir, "conf" + File.separator + "backup");
    this.fileSystem = fileSystem;
  }
    

  private XmlConfig loadConfig() throws ConfigException, AdminException {
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
    } catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  void save() throws AdminException {
    try {
      XmlConfig config = loadConfig();
      commonSettings.save(config);
      for (InstanceSettings is : instanceSettings) {
        is.save(config);
      }
      XmlConfigHelper.saveXmlConfig(config, smscConfigFile, backupDir, fileSystem);
    } catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
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

  public void setInstanceSettings(InstanceSettings instanceSettings) {
    int instanceNumber = instanceSettings.getInstanceN();
    if (instanceNumber < 0 || instanceNumber > this.instanceSettings.length - 1) {
      throw new IllegalArgumentException("Illegal instance number: " + instanceNumber + ". Must be in [0," + (this.instanceSettings.length - 1) + "]");
    }

    this.instanceSettings[instanceNumber] = instanceSettings;
  }
}
