/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:41
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.HashSet;
import java.util.Set;

public class ProfileFilter implements Filter
{
	private MaskList masks = new MaskList();
	private byte codepage = -1;
	private byte reportinfo = -1;
	private Set locales = new HashSet();

	public boolean isEmpty()
	{
		return masks.isEmpty() && codepage < 0 && reportinfo < 0 && locales.isEmpty();
	}

	public boolean isItemAllowed(DataItem item)
	{
		try
		{
			Profile p = new Profile(new Mask((String) item.getValue("Mask")), (String) item.getValue("Codepage"), (String) item.getValue("Report info"), (String) item.getValue("locale"));
			return (masks.isEmpty() || masks.contains(p.getMask())) && (codepage == -1 || codepage == p.getCodepage()) && (reportinfo == -1 || reportinfo == p.getReportOptions()) && (locales.isEmpty() || locales.contains(p.getLocale()));
		}
		catch (AdminException e)
		{
			Category.getInstance(this.getClass()).error("Exception", e);
			return false;
		}
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

	public Set getLocales()
	{
		return locales;
	}

	public void setLocales(Set locales)
	{
		this.locales = locales;
		if (this.locales == null)
			this.locales = new HashSet();
	}
}
