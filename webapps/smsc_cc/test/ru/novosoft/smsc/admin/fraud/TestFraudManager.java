package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Тестовая реализация FraudManager
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  private static byte[] currentConfig;

  static {
    InputStream is = null;
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    try {
      is = TestFraudManager.class.getResourceAsStream("fraud.xml");

      int b;
      while ((b = is.read()) > 0)
        os.write(b);

      currentConfig = os.toByteArray();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }

  public TestFraudManager() throws AdminException {
    super(null, null, null, null);
  }

  public synchronized void reset() throws AdminException {
    try {
      reset(new ByteArrayInputStream(currentConfig));
    } catch (Exception e) {
      throw new FraudException("read_config_error", e);
    }
  }

  public synchronized void apply() throws AdminException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    apply(os);
    currentConfig = os.toByteArray();
  }
}
