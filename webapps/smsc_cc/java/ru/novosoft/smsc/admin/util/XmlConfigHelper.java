package ru.novosoft.smsc.admin.util;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Класс, содержащие вспомогательные методы для роботы с XmlConfig
 * @author Artem Snopkov
 */
public class XmlConfigHelper {

  private static final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy.HH.mm.ss");

  /**
   * Сохраняет экземпляр XmlConfig в указанный файл. Перед сохранением создается бекап старой версии конфига в
   * директорию backupDir.
   *
   * @param config экземпляр XmlConfig, который надо сохранить.
   * @param toFile файл, в который надо сохранить конфиг
   * @param backupDir директория, в которую сохраняется бекап старого конфига или null, если бекап создавать не надо.
   * @param fileSystem экземпляр FileSystem
   * @throws AdminException если сохранить конфиг не удалось.
   */
  public static void saveXmlConfig(XmlConfig config, File toFile, File backupDir, FileSystem fileSystem) throws AdminException {

    if (backupDir != null) {
      if (!fileSystem.exists(backupDir))
        fileSystem.mkdirs(backupDir);
      File backupFile = new File(backupDir, "configFile.xml." + sdf.format(new Date()));
      if (fileSystem.exists(toFile))
        fileSystem.copy(toFile, backupFile);
    }

    File tmp = new File(toFile.getAbsolutePath() + ".tmp");
    OutputStream os = null;
    try {
      os = fileSystem.getOutputStream(tmp);
      config.save(os);
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }

    fileSystem.rename(tmp, toFile);
  }
}
