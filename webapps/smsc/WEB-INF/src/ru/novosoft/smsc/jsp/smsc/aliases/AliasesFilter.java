/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class AliasesFilter extends SmscBean
{
	protected AliasFilter filter = null;

	protected String[] aliases = null;
	protected String[] addresses = null;
	protected byte hide = AliasFilter.HIDE_UNKNOWN;

	protected String mbApply = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = appContext.getUserPreferences().getAliasesFilter();

		if (hide == AliasFilter.HIDE_UNKNOWN)
		{
			aliases = filter.getAliases();
			addresses = filter.getAddresses();
			hide = filter.getHide();
		}
		if (aliases == null)
			aliases = new String[0];
		if (addresses == null)
			addresses = new String[0];

		aliases = trimMasks(aliases);
		addresses = trimMasks(addresses);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			filter.setAddresses(addresses);
			filter.setAliases(aliases);
			filter.setHide(hide);
			return RESULT_DONE;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		return RESULT_OK;
	}


	protected String[] trimMasks(String[] masks)
	{
		Set newMasks = new HashSet();
		for (int i = 0; i < masks.length; i++)
		{
			String mask = masks[i];
			if (mask != null)
			{
				final String m = mask.trim();
				if (m.length() > 0 && !newMasks.contains(m))
					newMasks.add(m);
			}
		}
		return (String[]) newMasks.toArray(new String[0]);
	}


	/***************************** properties **********************************/
	public String[] getAliases()
	{
		return aliases;
	}

	public void setAliases(String[] aliases)
	{
		this.aliases = aliases;
	}

	public String[] getAddresses()
	{
		return addresses;
	}

	public void setAddresses(String[] addresses)
	{
		this.addresses = addresses;
	}

	public byte getHide()
	{
		return hide;
	}

	public void setHide(byte hide)
	{
		this.hide = hide;
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
