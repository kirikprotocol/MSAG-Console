package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Класс, предназначенный для итерирования по алиасам. После окончания использования должен быть вызван метод close()
 * @author Artem Snopkov
 */
public interface AliasSet {

  /**
   * Перемещает итератор на следующий алиас.
   * @return false, если достигнут конец списка
   */
  public boolean next();

  /**
   * Возвращает текущий алиас
   * @return текущий алиас
   */
  public Alias get();

  /**
   * Метод close() должен быть вызван после окончания работы с AliasSet
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  public void close() throws AdminException;

}
