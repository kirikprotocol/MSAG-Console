/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 5:42:31 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl.category;

import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.provider.Provider;
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
