package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/**
 * @author Artem Snopkov
 */
class TestRescheduleConfig extends RescheduleConfig {

  private static XmlConfig currentConfig;

  static {
    InputStream is = null;
    try {
      is = TestRescheduleConfig.class.getResourceAsStream("schedule.xml");
      currentConfig = new XmlConfig();
      currentConfig.load(is);
    } catch (XmlConfigException e) {
      e.printStackTrace();
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }

  public TestRescheduleConfig() {
    super(null, null, null);
  }

  void load() throws AdminException {
    try {
      load(currentConfig);
    } catch (XmlConfigException e) {
      throw new RescheduleException("invalid_config_file_format", e);
    }
  }

  void save() throws AdminException {
    try {
      save(currentConfig);
    } catch (XmlConfigException e) {
      throw new RescheduleException("config_file_save_error", e);
    }
  }
}
