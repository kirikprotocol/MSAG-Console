package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Абстрактный источник данных
 *
 * @author Aleksandr Khalitov
 */
public interface DataSource<T> {

  /**
   * Возвращает следующий объект. Если данных больше нет, то возвращает null
   *
   * @return следующий объект, если он есть, null иначе
   * @throws AdminException ошибка извлечения
   */
  public T next() throws AdminException;

}
