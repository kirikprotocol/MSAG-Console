package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/**
 * @author Artem Snopkov
 */
class TestSmscConfigFile extends SmscConfigFile {

  private static XmlConfig currentConfig;

  static {
    InputStream is = null;
    try {
      is = TestSmscConfigFile.class.getResourceAsStream("config.xml");
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

  public TestSmscConfigFile() {
    super(null, null, null);
  }

  void load() throws AdminException {
    try {
      load(currentConfig);
    } catch (XmlConfigException e) {
      throw new SmscException("invalid_config_file_format", e);
    }
  }

  void save() throws AdminException {
    try {
      save(currentConfig);
    } catch (XmlConfigException e) {
      throw new SmscException("save_error", e);
    }
  }
}
