package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 16:59:20
 *
 * Интерфейс абстрактного файлового ресурса, откуда можно закачивать файлы и куда их можно аплоадить
 */
interface FileResource {

  /**
   * Открывает доступ к файловому ресурсу.
   * @throws AdminException ошибка
   */
  public void open() throws AdminException;

  /**
   * Возвращает список csv файлов на файловом ресурсе
   * @return  список csv файлов на файловом ресурсе
   * @throws AdminException ошибка
   */
  public List<String> listCSVFiles() throws AdminException;

  /**
   * Загружает файл с ресурса
   * @param path путь к файлу на ресурсе
   * @param localFile путь для загрузки
   * @throws AdminException ошибка
   */
  public void get(String path, File localFile)  throws AdminException;

  /**
   * Переименовывает файл на файловом ресурсе
   * @param fromPath текущее имя
   * @param toPath новое имя
   * @throws AdminException ошибка
   */
  public void rename(String fromPath, String toPath) throws AdminException;

  /**
   * Удаляет файл на файловом ресурсе
   * @param path путь к файлу на ресурсе
   * @throws AdminException ошибка
   */
  public void remove(String path) throws AdminException;

  /**
   * Аплоадит файл на файловый ресурс
   * @param localFile локальный файл
   * @param toPath путь на ресурсе, куда надо загрузить файл
   * @throws AdminException ошибка
   */
  public void put(File localFile, String toPath) throws AdminException;

  /**
   * Закрывает доступ к файловому ресурсу
   * @throws AdminException ошибка
   */
  public void close() throws AdminException;

}
