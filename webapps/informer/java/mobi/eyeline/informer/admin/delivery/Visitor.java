package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Визитер для извлечения данных
 *
 * @param <T> тип извлекаемых данных
 */
public interface Visitor<T> {

  public boolean visit(T value) throws AdminException;

}
