package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;

/**
 * Структура, содержащая данные о провайдере
 * @author Artem Snopkov
 */
public class Provider implements Serializable {

  private static transient final ValidationHelper vh = new ValidationHelper(Provider.class);

  private final long id;
  private String name;

  Provider(long id, String name) throws AdminException {
    this.id = id;
    setName(name);
  }

  Provider(Provider copy) {
    this.id = copy.id;
    this.name = copy.name;
  }

  /**
   * Возвращает уникальный идентификатор провайдера
   * @return уникальный идентификатор провайдера
   */
  public long getId() {
    return id;
  }

  /**
   * Возвращает уникальное имя провайдера
   * @return уникальное имя провайдера
   */
  public String getName() {
    return name;
  }

  /**
   * Задает имя провайдера
   * @param name новое имя провайдера
   * @throws AdminException если имя некорректно
   */
  public void setName(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Provider provider = (Provider) o;

    return id == provider.id;
  }

  @Override
  public int hashCode() {
    return (int) (id ^ (id >>> 32));
  }
}
