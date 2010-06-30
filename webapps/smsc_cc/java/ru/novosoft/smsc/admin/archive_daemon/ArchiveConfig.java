package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.ConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * ����� ��� ������ � ������������� ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */

@SuppressWarnings({"EmptyCatchBlock"})
public class ArchiveConfig {

  private static final Logger logger = Logger.getLogger(ArchiveConfig.class);

  private final File configFile;

  private XmlConfig config = null;
  private String host;
  private short port;

  /**
   * @param host   ����, �� ������ ������� ArchiveDemon
   * @param config ���������������� ����
   * @throws AdminException ������ ��� ������ ����������������� �����
   */
  ArchiveConfig(String host, File config) throws AdminException {
    resetConfig(true);
    this.configFile = config;
    this.host = host;
  }

  /**
   * ������ �������
   *
   * @param load true -���������� ������ �� �����, false - �� ������
   * @throws AdminException ������ ��� ������ ����� ��� �������
   */
  public void resetConfig(boolean load) throws AdminException {
    try {
      if (load) {
        InputStream is = null;
        try {
          is = FileSystem.getInstance().getInputStream(configFile);
          config = new XmlConfig(is);
        } finally {
          if (is != null) {
            try {
              is.close();
            } catch (IOException e) {
            }
          }
        }
      }
      XmlConfigSection s = config.getSection("ArchiveDaemon").getSection("View");
      host = s.getString("host");
      port = (short) s.getInt("port");
    }
    catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  /**
   * ���������� ������� � ����
   *
   * @param config ������ ��� ����������
   * @throws AdminException ������ ��� ������
   */
  public void saveConfig(XmlConfig config) throws AdminException {
    OutputStream os = null;
    try {
      os = FileSystem.getInstance().getOutputStream(configFile);
      config.save(os);
    } catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException("Couldn't store SMSC config: " + e.getMessage());
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }
    this.config = config;
    resetConfig(false);
  }

  /**
   * ���������� ���� ������ ArchiveDemon
   *
   * @return ������ ArchiveDemon
   */
  public XmlConfig getConfig() {
    return config;
  }

  /**
   * ���������� ���� ArchiveDemon
   *
   * @return ����
   */
  public String getHost() {
    return host;
  }

  /**
   * ���������� ���� ArchiveDemon
   *
   * @return port
   */
  public short getPort() {
    return port;
  }
}