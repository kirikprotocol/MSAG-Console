package ru.novosoft.smsc.web.controllers.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class CategoryController extends SettingsMController<CategorySettings> {

  private List<Category> categories;
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
    for (Category c : categories) {
      Category cat = s.getCategory(c.getId());
      cat.setName(c.getName());
      s.updateCategory(cat);
    }
  }

  private void resetCategories() {
    categories = new LinkedList<Category>();
    for (Category c : getSettings().getCategories())
      categories.add(c);
  }


  public List<Category> getCategories() {
    return categories;
  }

  public String addCategory() {
    try {
      CategorySettings s = getSettings();
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
