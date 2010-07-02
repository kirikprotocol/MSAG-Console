package ru.novosoft.smsc.admin.filesystem;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.InstallationType;

import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * API для чтения/записи файлов
 *
 * @author Aleksandr Khalitov
 */
public abstract class FileSystem {

  protected static final Category logger = Category.getInstance(FileSystem.class);

  private static FileSystem instance;

  private static final Lock initLock = new ReentrantLock();

  public static FileSystem getInstance() throws AdminException{
    if(instance == null) {
      AdminContext context = AdminContext.getInstance();
      InstallationType installationType = context.getInstallationType();
      try{
        initLock.lock();
        if(instance == null) {
          switch (installationType) {
            case HS: instance = new FileSystemHS(context.getAppBaseDir(), context.getAppMirrorDirs()); break;
            default: instance = new FileSystemSingleHA();
          }
        }
      }finally {
        initLock.unlock();
      }
    }
    return instance;
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

}
