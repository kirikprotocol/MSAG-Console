/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:57:09
 */
package ru.novosoft.smsc.jsp.smsc;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.List;

public class SmscBean extends PageBean
{
	protected Smsc smsc = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		smsc = appContext.getSmsc();
		return RESULT_OK;
	}
}
