/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 17:59:20
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class ProfileDataItem extends AbstractDataItem
{
	protected ProfileDataItem(Profile profile) throws AdminException
	{
		values.put("Mask", profile.getMask().getMask());
		values.put("Codepage", profile.getCodepageString());
		values.put("Report info", profile.getReportOptionsString());
		values.put("locale", profile.getLocale());
	}
}
