/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.List;

public class SubjectsEdit extends SmscBean
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

		if (name == null || name.length() <= 0)
			return error(SMSCErrors.error.subjects.subjNotSpecified);

		if (defSme == null || masks == null)
		{
			Subject s = routeSubjectManager.getSubjects().get(name);
			defSme = s.getDefaultSme().getId();
			masks = (String[]) s.getMasks().getNames().toArray(new String[0]);
		}

		masks = trimStrings(masks);

		return result;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		if (mbCancel != null)
			return RESULT_DONE;

		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		try
		{
			if (masks == null || masks.length <= 0)
			{
				return error(SMSCErrors.error.subjects.masksNotDefined);
			}
			Subject s = routeSubjectManager.getSubjects().get(name);
			if (s == null)
				return error(SMSCErrors.error.subjects.subjNotFound, name);

			final SME defaultSme = smeManager.getSmes().get(defSme);
			if (defaultSme == null)
				return error(SMSCErrors.error.subjects.defaultSmeNotFound, defSme);
			s.setDefaultSme(defaultSme);
			s.setMasks(new MaskList(masks));
			appContext.getStatuses().setSubjectsChanged(true);
			return RESULT_DONE;
		}
		catch (Throwable e)
		{
			logger.debug("Can't save subject \"" + name + "\"", e);
			return error(SMSCErrors.error.subjects.cantEdit, name, e);
		}
	}

	public List getPossibleSmes()
	{
		return smeManager.getSmeNames();
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
