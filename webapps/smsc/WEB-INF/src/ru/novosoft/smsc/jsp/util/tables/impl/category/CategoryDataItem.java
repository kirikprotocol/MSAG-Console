package ru.novosoft.smsc.jsp.util.tables.impl.category;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class CategoryDataItem extends AbstractDataItem
{
  public CategoryDataItem(Category category)
  {
    values.put("id", String.valueOf(category.getId()));
    values.put("name", category.getName());
  }

  public String getId()
  {
    return (String) values.get("id");
  }

  public String getName()
  {
    return (String) values.get("name");
  }

}
