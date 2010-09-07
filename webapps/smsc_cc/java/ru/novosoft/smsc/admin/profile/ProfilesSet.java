package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Класс, предназначенный для итерирования по профилям. После окончания использования должен быть вызван метод close()
 * @author Artem Snopkov
 */
public interface ProfilesSet {

  /**
   * Перемещает итератор на следующий профиль.
   * @return false, если достигнут конец списка
   */
  public boolean next();

  /**
   * Возвращает текущий профиль
   * @return текущий профиль
   */
  public Profile get();

  /**
   * Метод close() должен быть вызван после окончания работы с ProfilesSet
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  public void close() throws AdminException;
}
