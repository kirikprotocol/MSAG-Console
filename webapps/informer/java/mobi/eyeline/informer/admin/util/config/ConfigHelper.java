package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Класс, содержащие вспомогательные методы для роботы с XmlConfig
 * @author Artem Snopkov
 */
public class ConfigHelper {

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
   * @throws mobi.eyeline.informer.util.config.XmlConfigException ошибка записи в конфиг
   */
  @SuppressWarnings({"EmptyCatchBlock"})
  public static void saveXmlConfig(XmlConfig config, File toFile, File backupDir, FileSystem fileSystem) throws AdminException, XmlConfigException {

    if (backupDir != null) {
      createBackup(toFile, backupDir, fileSystem);
    }

    File tmp = new File(toFile.getAbsolutePath() + ".tmp");
    OutputStream os = null;
    try {
      os = fileSystem.getOutputStream(tmp, false);
      config.save(os);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }

    fileSystem.delete(toFile);

    fileSystem.rename(tmp, toFile);
  }

  /**
   * Создаёт бэкап файла
   * @param file имя файла
   * @param backupDir директория для бэкапа
   * @param fileSystem файловая система
   * @return файл бэкапа
   * @throws AdminException ошибка при создании файла
   */
  public static File createBackup(File file, File backupDir, FileSystem fileSystem) throws AdminException {
    if (!fileSystem.exists(backupDir)) {
      fileSystem.mkdirs(backupDir);
    }
    File backupFile = new File(backupDir, file.getName() + "." + sdf.format(new Date()));
    if (fileSystem.exists(file)) {
      fileSystem.copy(file, backupFile);
    }
    return backupFile;
  }

  /**
   * Восстанавливает файл из бэкапа
   * @param file имя файла
   * @param configFile файл конфига
   * @param fileSystem файловая система
   * @throws AdminException ошибка при создании файла
   * @return true - восстановление прошло успешно, false - иначе
   */
  public static boolean rollbackConfig(File file, File configFile, FileSystem fileSystem) throws AdminException {
    if (fileSystem.exists(file)) {
      fileSystem.copy(file, configFile);
      return true;
    }
    return false;
  }


}
