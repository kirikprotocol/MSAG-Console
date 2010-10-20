package ru.novosoft.smsc.web.controllers.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

/**
 * @author Artem Snopkov
 */
public class CategoryController extends SettingsMController<CategorySettings> {

  private Map<Long, String> categories;
  private String newCategory;
  private boolean initError;

  public CategoryController() {
    super(WebContext.getInstance().getCategoryManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
      return;
    }
    resetCategories();
  }

  public boolean isInitError() {
    return initError;
  }

  private void updateCategories(CategorySettings s) throws AdminException {
    for (Long categoryId : categories.keySet()) {
      Category cat = s.getCategory(categoryId);
      cat.setName(categories.get(categoryId));
      s.updateCategory(cat);
    }
  }

  private void resetCategories() {
    categories = new TreeMap<Long, String>();
    for (Category c : getSettings().getCategories())
      categories.put(c.getId(), c.getName());
  }

  public Collection<Long> getIds() {
    return categories.keySet();
  }

  public Map<Long, String> getCategories() {
    return categories;
  }

  public String addCategory() {
    try {
      CategorySettings s = getSettings();
      updateCategories(s);
      s.addCategory(newCategory);
      setSettings(s);
      resetCategories();
      newCategory = null;
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String submit() {
    try {
      CategorySettings s = getSettings();
      updateCategories(s);
      setSettings(s);

      submitSettings();      

    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String reset() {
    try {
      resetSettings();
      resetCategories();
    } catch (AdminException e) {
      addError(e);
    }
    return "CATEGORIES";
  }

  public String getNewCategory() {
    return newCategory;
  }

  public void setNewCategory(String newCategory) {
    this.newCategory = newCategory;
  }
}
