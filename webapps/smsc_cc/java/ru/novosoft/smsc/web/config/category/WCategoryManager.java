package ru.novosoft.smsc.web.config.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

/**
 * @author Artem Snopkov
 */
public class WCategoryManager extends BaseSettingsManager<CategorySettings> implements CategoryManager {

  private final CategoryManager wrapped;
  private final Journal j;

  public WCategoryManager(CategoryManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  public CategorySettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public void _updateSettings(CategorySettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(CategorySettings oldSettings, CategorySettings newSettings) {
    for (Category c : newSettings.getCategories()) {
      Category oc = oldSettings.getCategory(c.getId());
      if (oc == null)
        j.user(user).add().category(c.getName(), c.getId() + "");
      else if (!c.getName().equals(oc.getName()))
        j.user(user).change("renamed", oc.getName(), c.getName()).category(c.getId() + "");
    }
  }

  public CategorySettings cloneSettings(CategorySettings settings) {
    return settings.cloneSettings();
  }
}
