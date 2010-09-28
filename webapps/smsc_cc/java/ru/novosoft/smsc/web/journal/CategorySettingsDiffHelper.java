package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategorySettings;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class CategorySettingsDiffHelper extends DiffHelper {

  public CategorySettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(CategorySettings oldSettings, CategorySettings newSettings, String user, Journal j) {
    for (Category c : newSettings.getCategories()) {
      Category oc = oldSettings.getCategory(c.getId());
      if (oc == null) {
        JournalRecord r = j.addRecord(JournalRecord.Type.ADD, Journal.CATEGORY, user);
        r.setDescription("category.added", c.getName(), c.getId() + "");
      } else if (!c.getName().equals(oc.getName())) {
        JournalRecord r = j.addRecord(JournalRecord.Type.CHANGE, Journal.CATEGORY, user);
        r.setDescription("category.renamed", c.getId() + "", oc.getName(), c.getName());
      }
    }
  }
}
