package ru.novosoft.smsc.admin.smsc_config;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.ConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;

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

  private XmlConfig config = new XmlConfig();

  private CommonSettings commonSettings;

  private InstanceSettings[] instanceSettings;

  void load(InputStream is) throws AdminException {
    try {
      config.clear();
      config.load(is);
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

  void save(OutputStream os) throws AdminException {
    try {
      commonSettings.save(config);
      for (InstanceSettings is : instanceSettings) {
        is.save(config);
      }
      config.save(os);
    } catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  void backup(OutputStream os) throws AdminException {
    try {
      config.save(os);
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
