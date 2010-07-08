package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.IOException;
import java.io.InputStream;

/**
 * Тестовая реализация ArchiveDaemonConfig
 * @author Artem Snopkov
 */
public class TestArchiveDaemonConfig extends ArchiveDaemonConfig {

  private final XmlConfig currentConfig;

  public TestArchiveDaemonConfig() throws AdminException {
    currentConfig = new XmlConfig();

    InputStream is = null;
    try {
      is = TestArchiveDaemonConfig.class.getResourceAsStream("config.xml");
      currentConfig.load(is);
    } catch (XmlConfigException e) {
      throw new ArchiveDaemonException("invalid_config_format", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }

    reset();
  }

  public void reset() throws AdminException {
    try {
      reset(currentConfig);
    } catch (XmlConfigException e) {
      throw new ArchiveDaemonException("invalid_config_format", e);
    }
  }

  public void save() throws AdminException {
    try {
      save(currentConfig);
    } catch (XmlConfigException e) {
      throw new ArchiveDaemonException("save_config_error", e);
    }
  }


}
