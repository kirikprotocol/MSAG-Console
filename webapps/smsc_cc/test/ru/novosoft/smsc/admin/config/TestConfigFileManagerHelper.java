package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * @author Artem Snopkov
 */
public class TestConfigFileManagerHelper {

  private byte[] currentConfig;

  public TestConfigFileManagerHelper(InputStream is) throws AdminException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    try {
      int b;
      while ((b = is.read()) > 0)
        os.write(b);

      currentConfig = os.toByteArray();
    } catch (IOException e) {
      throw new ConfigException("load_error");
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }

  public void reset(ConfigFileManager manager) throws AdminException {
    try {
      manager.config.load(new ByteArrayInputStream(currentConfig));
    } catch (Exception e) {
      throw new ConfigException("load_error");
    }
    manager.changed=false;
  }

  public void apply(ConfigFileManager manager) throws AdminException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    try {
      manager.config.save(new ByteArrayInputStream(currentConfig), os);
    } catch (Exception e) {
      throw new ConfigException("save_error", e);
    }
    currentConfig = os.toByteArray();
    manager.changed = false;
  }

}
