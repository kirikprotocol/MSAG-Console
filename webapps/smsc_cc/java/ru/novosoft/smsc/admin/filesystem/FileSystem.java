package ru.novosoft.smsc.admin.filesystem;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;

import java.io.*;

/**
 * API для чтения/записи файлов
 *
 * @author Aleksandr Khalitov
 */
public abstract class FileSystem {

  protected static final Category logger = Category.getInstance(FileSystem.class);


  public static FileSystem getInstance() {
    return null;                            //todo
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
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  /**
   * По названию файла, возвращает выходной поток
   *
   * @param file файл
   * @return поток
   * @throws AdminException ошибка ввода/вывода
   */
  public abstract OutputStream getOutputStream(File file) throws AdminException;

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
   * Удаляет файла
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

}
