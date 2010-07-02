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
 * todo Так как конфигурацию ArchiveDaemon можно править с веб-морды, этот класс надо причесать.
 * todo Надо сделать по аналогии с SmscConfig т.е. для всех параметров сделать геттеры и сеттеры.
 * todo Список параметров, выносимых на морду можно посмотреть на тестовой установке:
 * todo http://niagara:25011/smsc/smsc/smsview/daemon.jsp
 * todo Формат конфигурационного файла можно найти в CVS в smsc/config/templates/archiver/daemon.xml
 * Класс для работы с конфигурацией ArchiveDemon
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
   * @param host   хост, на которм запущен ArchiveDemon
   * @param config конфигурациооный файл
   * @throws AdminException ошибка при чтении конфигурационного файла
   */
  ArchiveConfig(String host, File config) throws AdminException {
    resetConfig(true);
    this.configFile = config;
    this.host = host;
  }

  /**
   * Чтение конфига
   *
   * @param load true -перечитать конфиг из файла, false - из памяти
   * @throws AdminException ошибка при чтении файла или конфига
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
   * todo Непонятный метод. Он сохраняет абстрактный XmlConfig (не обязательно относящийся к ArchiveDaemon)
   * todo в конфигурационный файл ArchiveDaemon.
   * todo Еще не делается бекап и сохранение происходит без tmp файла
   * Сохранение конфига в файл
   *
   * @param config конфиг для сохранения
   * @throws AdminException ошибка при записи
   */
  public void saveConfig(XmlConfig config) throws AdminException {
    OutputStream os = null;
    try {
      os = FileSystem.getInstance().getOutputStream(configFile);
      config.save(os);
    } catch (ConfigException e) {
      logger.error(e, e);
      throw new AdminException("Couldn't store SMSC config: " + e.getMessage());     // todo неверный текст
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
   * Возвращает весь конфиг ArchiveDemon
   *
   * @return конфиг ArchiveDemon
   */
  public XmlConfig getConfig() {
    return config;
  }

  /**
   * Возвращает хост ArchiveDemon
   *
   * @return хост
   */
  public String getHost() {
    return host;
  }

  /**
   * Возвращает хост ArchiveDemon
   *
   * @return port
   */
  public short getPort() {
    return port;
  }
}
