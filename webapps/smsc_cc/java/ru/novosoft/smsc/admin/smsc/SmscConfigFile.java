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
class SmscConfigFile implements ManagedConfigFile<SmscSettings> {

  protected SmscSettings load(XmlConfig config) throws XmlConfigException, AdminException {
    SmscSettings s = new SmscSettings();

    CommonSettings commonSettings = new CommonSettings(config);
    commonSettings.load(config);

    s.setCommonSettings(commonSettings);

    int instancesCount = config.getSection("cluster").getInt("nodesCount");
    InstanceSettings[] instanceSettings = new InstanceSettings[instancesCount];
    for (int i = 0; i < instancesCount; i++) {
      instanceSettings[i] = new InstanceSettings(i, config);
    }
    s.setInstancesSettings(instanceSettings);

    return s;
  }

  public SmscSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);
    return load(config);
  }

  protected void save(XmlConfig config, SmscSettings settings) throws XmlConfigException {
    settings.getCommonSettings().save(config);
    for (int i = 0; i < settings.getSmscInstancesCount(); i++) {
      settings.getInstanceSettings(i).save(i, config);
    }
  }

  public void save(InputStream is, OutputStream os, SmscSettings settings) throws Exception {
    XmlConfig config = new XmlConfig(is);
    save(config, settings);
    config.save(os);
  }


}
