/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.List;
import java.util.Iterator;

public class AliasesAdd extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

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

	private int countQuestions(String str)
	{
		final int start = str.indexOf('?');
		if (start >= 0)
			return str.lastIndexOf('?') - start + 1;
		else
			return 0;
	}

	protected int save()
	{
		if (!Mask.isMaskValid(address))
			return error(SMSCErrors.error.aliases.invalidAddress, address);
		if (!Mask.isMaskValid(alias))
			return error(SMSCErrors.error.aliases.invalidAlias, alias);
		if (isHide() && (address.indexOf('?') >= 0 || alias.indexOf('?') >= 0))
			return error(SMSCErrors.error.aliases.HideWithQuestion);
		if (countQuestions(address) != countQuestions(alias))
			return error(SMSCErrors.error.aliases.QuestionCountsNotMathes);

		AliasSet aliases = smsc.getAliases();
      if (isHide())
		{
         for (Iterator i = aliases.iterator(); i.hasNext();)
			{
				Alias alias = (Alias) i.next();
				if (alias.isHide() && alias.getAddress().getMask().equals(address))
					return error(SMSCErrors.error.aliases.alreadyExistsAddress, address);
			}
		}

		try
		{
			if (aliases.add(new Alias(new Mask(address), new Mask(alias), hide)))
			{
				appContext.getStatuses().setAliasesChanged(true);
				return RESULT_DONE;
			}
			else
				return error(SMSCErrors.error.aliases.alreadyExists, alias);
		}
		catch (Throwable t)
		{
			logger.error("Couldn't add alias \"" + address + "\"-->\"" + alias + "\"", t);
			return error(SMSCErrors.error.aliases.cantAdd, alias);
		}
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
