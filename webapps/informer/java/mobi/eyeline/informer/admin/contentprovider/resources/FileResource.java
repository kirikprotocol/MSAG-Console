package mobi.eyeline.informer.admin.contentprovider.resources;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 16:59:20
 *
 * Интерфейс абстрактного файлового ресурса, откуда можно закачивать файлы и куда их можно аплоадить
 */
public abstract class FileResource {

  public static FileResource createLocal(File dir, FileSystem fs) {
    return new LogDecorator(new LocalResource(dir, fs));
  }

  public static FileResource createFTP(String host, Integer port, String login, String password, String remoteDir, boolean passiveMode) {
    return new LogDecorator(new FTPResource(host, port, login, password, remoteDir, passiveMode));
  }

  public static FileResource createSFTP(String host, Integer port, String login, String password, String remoteDir) {
    return new LogDecorator(new SFTPResource(host, port, login, password, remoteDir));
  }

  public static FileResource createSMB(String host, Integer port, String login, String password, String remoteDir, String domain) {
    return new LogDecorator(new SMBResource(host, port, login, password, remoteDir, domain));
  }

  public static FileResource createEmpty() {
    return new NullResource();
  }

  /**
   * Открывает доступ к файловому ресурсу.
   * @throws AdminException ошибка
   */
  public abstract void open() throws AdminException;

  /**
   * Возвращает список csv файлов на файловом ресурсе
   * @return  список csv файлов на файловом ресурсе
   * @throws AdminException ошибка
   */
  public abstract List<String> listCSVFiles() throws AdminException;

  /**
   * Загружает файл с ресурса
   * @param path путь к файлу на ресурсе
   * @param os поток для записи
   * @throws AdminException ошибка
   */
  public abstract void get(String path, OutputStream os)  throws AdminException;

  /**
   * Переименовывает файл на файловом ресурсе
   * @param fromPath текущее имя
   * @param toPath новое имя
   * @throws AdminException ошибка
   */
  public abstract void rename(String fromPath, String toPath) throws AdminException;

  /**
   * Удаляет файл на файловом ресурсе
   * @param path путь к файлу на ресурсе
   * @throws AdminException ошибка
   */
  public abstract void remove(String path) throws AdminException;

  /**
   * Аплоадит файл на файловый ресурс
   * @param is поток для чтения данных
   * @param toPath путь на ресурсе, куда надо загрузить файл
   * @throws AdminException ошибка
   */
  public abstract void put(InputStream is, String toPath) throws AdminException;

  /**
   * Закрывает доступ к файловому ресурсу
   * @throws AdminException ошибка
   */
  public abstract void close() throws AdminException;

}
