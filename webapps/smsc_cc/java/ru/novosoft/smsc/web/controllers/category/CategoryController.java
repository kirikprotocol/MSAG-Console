package ru.novosoft.smsc.web.controllers.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class CategoryController extends SettingsController<CategorySettings> {

  private Map<Long, String> categories;
  private String newCategory;

  public CategoryController() {
    super(ConfigType.Category);
    resetCategories();
    checkChanges();
  }

  private void checkChanges() {
    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.submit.hint");
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

  @Override
  protected CategorySettings loadSettings() throws AdminException {
    return getConfiguration().getCategorySettings();
  }

  @Override
  protected void saveSettings(CategorySettings settings) throws AdminException {
    getConfiguration().updateCategorySettings(settings, getUserName());
  }

  @Override
  protected CategorySettings cloneSettings(CategorySettings settings) {
    return settings.cloneSettings();
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
      checkChanges();
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

      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual", rev.getUser());
        return null;
      }

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
