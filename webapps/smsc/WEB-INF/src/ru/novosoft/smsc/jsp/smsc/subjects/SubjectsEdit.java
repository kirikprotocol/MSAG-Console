/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.List;
import java.util.Set;

public class SubjectsEdit extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	protected String oldName = null;

	protected String name = null;
	protected String defSme = null;
	protected String[] masks = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (name == null || name.length() <= 0)
			return error(SMSCErrors.error.subjects.subjNotSpecified);

		if (defSme == null || masks == null)
		{
			Subject s = smsc.getSubjects().get(name);
			defSme = s.getDefaultSme().getId();
			masks = (String[]) s.getMasks().getNames().toArray(new String[0]);
		}

		if (oldName == null)
			oldName = name;

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
		if (!name.equals(oldName) && smsc.getSubjects().contains(name))
			return error(SMSCErrors.error.subjects.alreadyExists, name);
		else
		{
			try
			{
				if (oldName.equals(name))
				{
					Subject s = smsc.getSubjects().get(name);
					s.setDefaultSme(smsc.getSmes().get(defSme));
					s.setMasks(new MaskList(masks));
				}
				else
				{
					smsc.getSubjects().remove(oldName);
					smsc.getSubjects().add(new Subject(name, masks, smsc.getSmes().get(defSme)));
				}
				appContext.getStatuses().setSubjectsChanged(true);
				return RESULT_DONE;
			}
			catch (Throwable e)
			{
				return error(SMSCErrors.error.subjects.cantEdit, name, e);
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

	public String getOldName()
	{
		return oldName;
	}

	public void setOldName(String oldName)
	{
		this.oldName = oldName;
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
