package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Вспомогательный класс для работы (загрузки/сохранения) с config.xml СМС центра.
 * Те пареметры, которые не отображаются на веб-интерфейсе, должны проноситься без изменений между load и save !!!
 *
 * @author Artem Snopkov
 */
class SmscConfigFile implements ManagedConfigFile {


  private CommonSettings commonSettings;

  private InstanceSettings[] instanceSettings;

  protected void load(XmlConfig config) throws XmlConfigException, AdminException {
    commonSettings = new CommonSettings(config);
    commonSettings.load(config);

    int instancesCount = config.getSection("cluster").getInt("nodesCount");
    instanceSettings = new InstanceSettings[instancesCount];
    for (int i = 0; i < instancesCount; i++) {
      instanceSettings[i] = new InstanceSettings(i, config);
    }
  }

  public void load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);
    load(config);
  }

  protected void save(XmlConfig config) throws XmlConfigException {
    commonSettings.save(config);
    for (int i = 0; i < instanceSettings.length; i++) {
      instanceSettings[i].save(i, config);
    }
  }

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig config = new XmlConfig(is);
    save(config);
    config.save(os);
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
