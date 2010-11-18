package mobi.eyeline.informer.admin.util.config;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Интерфейс для работы с конфигурационным файлом, используемый в ConfigFileManager
 *
 * @author Artem Snopkov
 */
public interface ManagedConfigFile<C> {

  /**
   * Сохраняет конфигурационный файл
   *
   * @param oldFile поток для чтения старого конфигурационного файла
   * @param newFile поток для записи нового конфигурационного файла
   * @param conf    экземпляр конфигурации, которую надо сохранить в файл
   * @throws Exception если произошла ошибка
   */
  void save(InputStream oldFile, OutputStream newFile, C conf) throws Exception;

  /**
   * Загружает конфигурационный файл из заданного потока
   *
   * @param is поток, содержащий конфигурационный файл
   * @return экземпляр конфигурации, загруженной из файла
   * @throws Exception если произошла ошибка
   */
  C load(InputStream is) throws Exception;

}
