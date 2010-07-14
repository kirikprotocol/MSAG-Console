package ru.novosoft.smsc.admin.config;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Интерфейс для работы с конфигурационным файлом, используемый в ConfigFileManager
 * @author Artem Snopkov
 */
public interface ManagedConfigFile {

  /**
   * Сохраняет конфигурационный файл
   * @param oldFile поток для чтения старого конфигурационного файла
   * @param newFile поток для записи нового конфигурационного файла
   * @throws Exception если произошла ошибка
   */
  void save(InputStream oldFile, OutputStream newFile) throws Exception;

  /**
   * Загружает конфигурационный файл из заданного потока
   * @param is поток, содержащий конфигурационный файл
   * @throws Exception если произошла ошибка
   */
  void load(InputStream is) throws Exception;

}
