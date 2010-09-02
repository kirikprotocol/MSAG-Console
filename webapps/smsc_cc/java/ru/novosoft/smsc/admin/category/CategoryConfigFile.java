package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
class CategoryConfigFile implements ManagedConfigFile<CategorySettings> {
  
  public void save(InputStream oldFile, OutputStream newFile, CategorySettings conf) throws Exception {
    XmlConfig cfg = new XmlConfig(oldFile);

    XmlConfigSection categoriesSection = cfg.getOrCreateSection("categories");
    categoriesSection.clear();

    for (Category p : conf.getCategories())
      categoriesSection.setString(String.valueOf(p.getId()), p.getName());

    categoriesSection.setLong("last used category id", conf.getLastCategoryId());

    cfg.save(newFile);
  }

  public CategorySettings load(InputStream is) throws Exception {
    XmlConfig cfg = new XmlConfig(is);

    if (cfg.containsSection("categories")) {
      Collection<Category> categories = new ArrayList<Category>();
      long lastUsedCategoryId = -1;
      XmlConfigSection CategorysSection = cfg.getSection("categories");
      for (XmlConfigParam p : CategorysSection.params()) {
        if (p.getName().equals("last used category id")) {
          lastUsedCategoryId = p.getLong();
        } else {
          categories.add(new Category(Long.parseLong(p.getName()), p.getString()));
        }
      }
      return new CategorySettings(categories, lastUsedCategoryId);
    }

    return new CategorySettings(new ArrayList<Category>(), -1);
  }
}
