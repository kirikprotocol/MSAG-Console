package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.AdminException;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Настройки категорий
 * @author Artem Snopkov
 */
public class CategorySettings implements Serializable {
  
  private final Map<Long, Category> categories;
  private long lastCategoryId;

  CategorySettings(Collection<Category> categories, long lastCategoryId) {
    this.categories = new HashMap<Long, Category>();
    this.lastCategoryId  = lastCategoryId;
    for (Category p : categories)
      this.categories.put(p.getId(), p);
  }

  CategorySettings(CategorySettings settings) {
    this(settings.getCategories(), settings.getLastCategoryId());
  }

  private void checkName(String name, Long currentCategoryId) throws AdminException {
    for (Category p : categories.values())
      if (p.getName().equals(name) && (currentCategoryId == null || currentCategoryId != p.getId()))
        throw new CategoryException("category.name.is.not.unique", name);
  }

  long getLastCategoryId() {
    return lastCategoryId;
  }

  /**
   * Добавляет в настройки и возвращает новой категории
   * @param name имя новой категории
   * @return инстанц Category
   * @throws AdminException если имя категории некорректно или неуникально
   */
  public Category addCategory(String name) throws AdminException {
    checkName(name, null);
    Category newCategory = new Category(++lastCategoryId, name);
    categories.put(newCategory.getId(), newCategory);
    return new Category(newCategory);
  }

  /**
   * Обновляет данные о категории
   * @param updatedCategory новые данные о категории
   * @throws AdminException если новое имя категории не уникально
   */
  public void updateCategory(Category updatedCategory) throws AdminException {
    checkName(updatedCategory.getName(), updatedCategory.getId());
    categories.put(updatedCategory.getId(), new Category(updatedCategory));
  }

  /**
   * Возвращает категорию по её идентификатору
   * @param id идентификатор категории
   * @return  категорию по её идентификатору или null, если категории нет
   */
  public Category getCategory(long id) {
    Category p = categories.get(id);
    if (p == null)
      return p;
    return new Category(p);
  }

  /**
   * Возвращает список категорий
   * @return список категорий
   */
  public Collection<Category> getCategories() {
    ArrayList<Category> result = new ArrayList<Category>(categories.size());
    for (Category p : categories.values())
      result.add(new Category(p));
    return result;
  }

  /**
   * Возвращает копию текущего инстанца CategorySettings
   * @return копию текущего инстанца CategorySettings
   */
  public CategorySettings cloneSettings() {
    return new CategorySettings(this);
  }
}
