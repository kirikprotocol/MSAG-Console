/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SmscBean;

import java.util.List;
import java.util.Set;

public class SubjectsAdd extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	protected String name = null;
	protected String defSme = null;
	protected String[] masks = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (name == null)
		{
			name = defSme = "";
			masks = new String[0];
		}

		if (masks == null)
			masks = new String[0];

		masks = trimStrings(masks);

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
		if (smsc.getSubjects().contains(name))
			return error(SMSCErrors.error.subjects.alreadyExists, name);
		else
		{
			try
			{
				smsc.getSubjects().add(new Subject(name, masks, smsc.getSmes().get(defSme)));
				return RESULT_DONE;
			}
			catch (Throwable e)
			{
				return error(SMSCErrors.error.subjects.cantAdd, name, e);
			}
		}
	}

	public Set getPossibleSmes()
	{
		return smsc.getSmes().getNames();
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

	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	public String getDefSme()
	{
		return defSme;
	}

	public void setDefSme(String defSme)
	{
		this.defSme = defSme;
	}

	public String[] getMasks()
	{
		return masks;
	}

	public void setMasks(String[] masks)
	{
		this.masks = masks;
	}
}
