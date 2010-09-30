package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface CategoryManager {

  /**
   * Возвращает настройки категорий
   * @return настройки категорий
   * @throws AdminException если произошла ошибка
   */
  public CategorySettings getSettings() throws AdminException;

  /**
   * Обновляет настройки категорий
   * @param settings новые настройки категорий
   * @throws AdminException если произошла ошибка
   */
  public void updateSettings(CategorySettings settings) throws AdminException;
}
