/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:41
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

public class ProfileFilter implements Filter
{
	private String[] masks = new String[0];
	private byte codepage = -1;
	private byte reportinfo = -1;

	public boolean isEmpty()
	{
		return masks.length == 0 && codepage < 0 && reportinfo < 0;
	}

	public boolean isItemAllowed(DataItem item)
	{
		return true;
	}

	public String[] getMasks()
	{
		return masks;
	}

	public void setMasks(String[] masks)
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
