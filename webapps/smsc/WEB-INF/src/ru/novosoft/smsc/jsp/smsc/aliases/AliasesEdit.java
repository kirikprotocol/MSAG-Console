/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.Iterator;
import java.util.List;

public class AliasesEdit extends SmscBean
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

		try
		{
			alias = new Mask(alias).getMask();
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.aliases.invalidAlias, alias);
		}
		try
		{
			address = new Mask(address).getMask();
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.aliases.invalidAddress, address);
		}

		if (oldAlias == null || oldAddress == null)
		{
			oldAlias = alias;
			oldAddress = address;
			oldHide = false;
		}

		return result;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (!Mask.isMaskValid(address))
			return error(SMSCErrors.error.aliases.invalidAddress, address);
		if (!Mask.isMaskValid(alias))
			return error(SMSCErrors.error.aliases.invalidAlias, alias);
		if (isHide() && (address.indexOf('?') >= 0 || alias.indexOf('?') >= 0))
			return error(SMSCErrors.error.aliases.HideWithQuestion);
		if (countQuestions(address) != countQuestions(alias))
			return error(SMSCErrors.error.aliases.QuestionCountsNotMathes);

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

	private static AliasSet getAliasesByAddressAndHide(AliasSet sourceSet, Mask address, boolean hide)
	{
		AliasSet result = new AliasSet();
		for (Iterator i = sourceSet.iterator(); i.hasNext();)
		{
			Alias alias = (Alias) i.next();
			if ((alias.isHide() == hide) && alias.getAddress().equals(address))
				result.add(alias);
		}
		return result;
	}

	protected int save()
	{
		Alias al;
		try
		{
			al = new Alias(new Mask(address), new Mask(alias), hide);
		}
		catch (AdminException e)
		{
			logger.error("Ureachable code reached!", e);
			return error(SMSCErrors.error.aliases.invalidAddress, address);
		}
		Alias oldAl;
		try
		{
			oldAl = new Alias(new Mask(oldAddress), new Mask(oldAlias), oldHide);
		}
		catch (AdminException e)
		{
			logger.debug("Error in old alias", e);
			return error(SMSCErrors.error.aliases.invalidAddress, oldAddress);
		}

		boolean aliasChanged = !oldAl.getAlias().equals(al.getAlias());
		boolean addressChanged = !oldAl.getAddress().equals(al.getAddress());

		if (aliasChanged && smsc.getAliases().contains(al))
			return error(SMSCErrors.error.aliases.alreadyExistsAlias, al.getAlias().getMask());

		if (al.isHide())
		{
			AliasSet byAddr = getAliasesByAddressAndHide(smsc.getAliases(), al.getAddress(), true);
			if (!addressChanged)
				byAddr.remove(oldAl);
			if (byAddr.size() > 0)
				return error(SMSCErrors.error.aliases.alreadyExistsAddress, al.getAddress().getMask());
		}

		try
		{
			smsc.getAliases().remove(oldAl);
			if (smsc.getAliases().add(al))
			{
				appContext.getStatuses().setAliasesChanged(true);
				return RESULT_DONE;
			}
			else
				return error(SMSCErrors.error.aliases.alreadyExistsAlias, alias);
		}
		catch (Throwable t)
		{
			logger.error("Couldn't edit alias \"" + address + "\"-->\"" + alias + "\"", t);
			return error(SMSCErrors.error.aliases.cantEdit, alias);
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
