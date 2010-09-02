package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;

/**
 * Структура, содержащая данные о категории
 * @author Artem Snopkov
 */
public class Category implements Serializable {
  
  private static transient final ValidationHelper vh = new ValidationHelper(Category.class);

  private final long id;
  private String name;

  Category(long id, String name) throws AdminException {
    this.id = id;
    setName(name);
  }

  Category(Category copy) {
    this.id = copy.id;
    this.name = copy.name;
  }

  /**
   * Возвращает уникальный идентификатор категории
   * @return уникальный идентификатор категории
   */
  public long getId() {
    return id;
  }

  /**
   * Возвращает уникальное имя категории
   * @return уникальное имя категории
   */
  public String getName() {
    return name;
  }

  /**
   * Задает имя категории
   * @param name новое имя категории
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

    Category Category = (Category) o;

    return id == Category.id;
  }

  @Override
  public int hashCode() {
    return (int) (id ^ (id >>> 32));
  }
}
