/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 5:42:31 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.users.User;

public class UserDataItem extends AbstractDataItem
{
	public UserDataItem(User user)
	{
		values.put("login", user.getLogin());
		values.put("firstName", user.getFirstName());
		values.put("lastName", user.getLastName());
		values.put("dept", user.getDept());
	}
}
