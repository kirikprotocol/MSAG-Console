package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public interface AliasManager extends SmscConfiguration {

  /**
   * Добавляет/обновляет алиас.
   *
   * @param alias алиас
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  public void addAlias(Alias alias) throws AdminException;

  /**
   * Удаляет алиас
   *
   * @param alias алиас
   * @throws AdminException если произошла ошибка
   */
  public void deleteAlias(Address alias) throws AdminException;

  /**
   * Возвращает экземпляр AliasSet, с помощью которого можно итерироваться по алиасам.
   *
   * @return экземпляр AliasSet, с помощью которого можно итерироваться по алиасам.
   * @throws AdminException если произошла ошибка
   */
  public AliasSet getAliases() throws AdminException;
}
