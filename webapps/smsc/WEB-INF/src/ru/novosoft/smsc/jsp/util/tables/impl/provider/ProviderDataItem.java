/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 5:42:31 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl.provider;

import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class ProviderDataItem extends AbstractDataItem
{
	public ProviderDataItem(Provider provider)
	{
		values.put("id", String.valueOf(provider.getId()));
		values.put("name", provider.getName());
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
