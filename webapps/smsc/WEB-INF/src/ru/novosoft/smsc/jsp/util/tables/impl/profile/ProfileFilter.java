/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:41
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

public class ProfileFilter implements Filter
{
	private MaskList masks = new MaskList();
	private byte codepage = -1;
	private byte reportinfo = -1;

	public boolean isEmpty()
	{
		return masks.isEmpty() && codepage < 0 && reportinfo < 0;
	}

	public boolean isItemAllowed(DataItem item)
	{
		return true;
	}

	public MaskList getMasks()
	{
		return masks;
	}

	public void setMasks(MaskList masks)
	{
		this.masks = masks;
	}

	public byte getCodepage()
	{
		return codepage;
	}

	public void setCodepage(byte codepage)
	{
		this.codepage = codepage;
	}

	public byte getReportinfo()
	{
		return reportinfo;
	}

	public void setReportinfo(byte reportinfo)
	{
		this.reportinfo = reportinfo;
	}
}
