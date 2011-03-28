package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;

import java.io.*;

/**
 * API для чтения/записи файлов
 *
 * @author Aleksandr Khalitov
 */
public abstract class FileSystem {

  public static FileSystem getFSForSingleInst() {
    return new FileSystemSingleHA();
  }

  public static FileSystem getFSForHAInst() {
    return new FileSystemSingleHA();
  }

  public static FileSystem getFSForHSInst(File baseDir, File[] mirrors) {
    return new FileSystemHS(baseDir, mirrors);

  }

  /**
   * По названию файла, возвращает входной поток
   *
   * @param file файл
   * @return поток
   * @throws AdminException ошибка ввода/вывода
   */
  public InputStream getInputStream(File file) throws AdminException {
    try {
      return new FileInputStream(file);
    } catch (IOException e) {
      throw new FileSystemException("io_error", e);
    }
  }

  /**
   * По названию файла, возвращает выходной поток
   *
   * @param file   файл
   * @param append Добавлять в файл или переписать
   * @return поток
   * @throws AdminException ошибка ввода/вывода
   */
  public abstract OutputStream getOutputStream(File file, boolean append) throws AdminException;

  /**
   * Переименовывает один файл в другой
   *
   * @param file   исходный файл
   * @param toFile новый файл
   * @throws AdminException ошибка при переименовании
   */
  public abstract void rename(File file, File toFile) throws AdminException;

  /**
   * Копирует содержимое одного файла в другой
   *
   * @param file   исходный файл
   * @param toFile приёмник
   * @throws AdminException ошибка при копировании
   */
  public abstract void copy(File file, File toFile) throws AdminException;

  /**
   * Удаляет файл
   *
   * @param file файл
   * @throws AdminException ошибка при удалении
   */
  public abstract void delete(File file) throws AdminException;

  /**
   * Создаёт новую директорию
   *
   * @param file директория
   * @throws AdminException ошибка при создании
   */
  public abstract void mkdirs(File file) throws AdminException;

  /**
   * Проверяет существование файла или директории
   *
   * @param file файл или директория
   * @return true - существует, false - иначе
   * @throws AdminException ошибка при проверки
   */
  public abstract boolean exists(File file) throws AdminException;

  /**
   * Возвращает список имён файлов в данной директории
   *
   * @param file директория
   * @return список имён файлов
   */
  public String[] list(File file) throws AdminException {
    return file.list();
  }

  /**
   * Возвращает список файлов в данной директории
   *
   * @param file директория
   * @return список файлов
   */
  public File[] listFiles(File file) {
    return file.listFiles();
  }

  /**
   * Возвращает список файлов в данной директории
   *
   * @param file директория
   * @param fileFilter фильтр
   * @return список файлов
   */
  public File[] listFiles(File file, FileFilter fileFilter) {
    return file.listFiles(fileFilter);
  }

  /**
   * Является ли данный File директорией
   *
   * @param file File
   * @return true - да, false - нет
   */
  public boolean isDirectory(File file) {
    return file.isDirectory();
  }

  /**
   * Возвращает размер файла
   * @param f File
   * @return размер файла в байтах
   * @throws AdminException ошибка ввода/вывода
   */
  public abstract long length(File f) throws AdminException;

  /**
   * Создаёт новый файл
   * @param f File
   * @throws AdminException ошибка ввода/вывода
   */
  public abstract void createNewFile(File f) throws AdminException;


  /**
   * Удаляет директорию и все её поддиректории
   *
   * @param folder директория
   * @throws AdminException ошибка ввода/вывода
   */
  public void recursiveDeleteFolder(File folder) throws AdminException {
    File[] childs = listFiles(folder);
    if (childs != null) {
      for (File child : childs) {
        if (isDirectory(child)) {
          recursiveDeleteFolder(child);
        } else {
          delete(child);
        }
      }
    }
    delete(folder);
  }
}
