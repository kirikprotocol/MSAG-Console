package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public interface ProfileManager extends SmscConfiguration {

  /**
   * Возвращает дефолтный профиль
   * @return дефолтный профиль
   * @throws AdminException если произошла ошибка
   */
  Profile getDefaultProfile() throws AdminException;

  /**
   * Обновляет дефолтный профиль
   * @param profile новые параметры дефолтного профиля
   * @throws AdminException
   */
  void updateDefaultProfile(Profile profile) throws AdminException;

  /**
   * Ищет профиль, максимально подходящий под указанный адрес(маску)
   * В возвращаемом объекте ProfileLookupResult поле Address равно null.
   * Таким образом, если произошло совпадение по маске, то узнать какой именно
   * маске соответствует профиль нельзя. 
   *
   * @param mask адрес или маска
   * @return профиль, максимально подходящий под данную маску
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  ProfileLookupResult lookupProfile(Address mask) throws AdminException;

  /**
   * Обновляет или создает новый профиль.
   *
   * @param profile профиль
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  void updateProfile(Profile profile) throws AdminException;

  /**
   * Удаляет профиль по его маске
   *
   * @param mask маска
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  void deleteProfile(Address mask) throws AdminException;

  /**
   * Возвращает экземпляр ProfilesSet, с помощью которого можно итерироваться по профилям
   *
   * @return экземпляр ProfilesSet, с помощью которого можно итерироваться по профилям
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  ProfilesSet getProfiles() throws AdminException;
}
