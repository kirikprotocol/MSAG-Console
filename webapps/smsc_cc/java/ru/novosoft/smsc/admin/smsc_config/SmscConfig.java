package ru.novosoft.smsc.admin.smsc_config;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Вспомогательный класс для работы (загрузки/сохранения) с config.xml СМС центра.
 * Те пареметры, которые не отображаются на веб-интерфейсе, должны проноситься без изменений между load и save !!!
 *
 * @author Artem Snopkov
 */
class SmscConfig {

  public void load(InputStream is) {

  }

  public void save(OutputStream os) {

  }

  public CommonSettings getCommonSettings() {
    return null;
  }

  public int getSmscInstancesCount() {
    return 0;
  }

  public InstanceSettings getInstanceSettings(int instanceNumber) {
    return null;
  }

  

}
