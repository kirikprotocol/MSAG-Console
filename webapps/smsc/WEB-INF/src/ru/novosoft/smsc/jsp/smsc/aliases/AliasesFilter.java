/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;

import java.util.List;

public class AliasesFilter extends SmscBean
{
	protected AliasFilter filter = null;

	protected String[] aliases = null;
	protected String[] addresses = null;
	protected byte hide = AliasFilter.HIDE_UNKNOWN;

	protected String mbApply = null;
	protected String mbClear = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = preferences.getAliasesFilter();

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

		try
		{
			aliases = MaskList.normalizeMaskList(trimStrings(aliases));
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.aliases.invalidAlias, e);
		}
		try
		{
			addresses = MaskList.normalizeMaskList(trimStrings(addresses));
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.aliases.invalidAddress, e);
		}

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		if (mbCancel != null)
			return RESULT_DONE;

		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			try
			{
				filter.setAddresses(addresses);
			}
			catch (AdminException e)
			{
				return error(SMSCErrors.error.aliases.invalidAddress, e);
			}
			try
			{
				filter.setAliases(aliases);
			}
			catch (AdminException e)
			{
				return error(SMSCErrors.error.aliases.invalidAlias, e);
			}
			filter.setHide(hide);
			return RESULT_DONE;
		}
		else if (mbClear != null)
		{
			aliases = addresses = new String[0];
			hide = AliasFilter.HIDE_UNKNOWN;
			return RESULT_OK;
		}

		return RESULT_OK;
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

	public String getMbClear()
	{
		return mbClear;
	}

	public void setMbClear(String mbClear)
	{
		this.mbClear = mbClear;
	}
}
