/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SmscBean;

import java.util.List;

public class AliasesAdd extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	protected String oldAlias = null;
	protected String oldAddress = null;
	protected boolean oldHide = false;

	protected String alias = null;
	protected String address = null;
	protected boolean hide = false;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (alias == null || address == null)
		{
			alias = address = "";
			hide = false;
		}

		if (oldAlias == null || oldAddress == null)
		{
			oldAlias = alias;
			oldAddress = address;
			oldHide = false;
		}

		return result;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;
		else if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		return smsc.getAliases().add(new Alias(new Mask(address), new Mask(alias), hide))
				  ? RESULT_DONE
				  : error(SMSCErrors.error.aliases.alreadyExists, alias);
	}

	/*************************** properties *********************************/

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public String getOldAlias()
	{
		return oldAlias;
	}

	public void setOldAlias(String oldAlias)
	{
		this.oldAlias = oldAlias;
	}

	public String getOldAddress()
	{
		return oldAddress;
	}

	public void setOldAddress(String oldAddress)
	{
		this.oldAddress = oldAddress;
	}

	public boolean isOldHide()
	{
		return oldHide;
	}

	public void setOldHide(boolean oldHide)
	{
		this.oldHide = oldHide;
	}

	public String getAlias()
	{
		return alias;
	}

	public void setAlias(String alias)
	{
		this.alias = alias;
	}

	public String getAddress()
	{
		return address;
	}

	public void setAddress(String address)
	{
		this.address = address;
	}

	public boolean isHide()
	{
		return hide;
	}

	public void setHide(boolean hide)
	{
		this.hide = hide;
	}
}
