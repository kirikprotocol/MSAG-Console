/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;

import java.util.*;

public class SubjectsFilter extends SmscBean
{
	protected SubjectFilter filter = null;

	protected String[] checkedSubjects = null;
	protected String[] checkedSmes = null;
	protected String[] masks = null;

	protected Set checkedSubjectsSet = null;
	protected Set checkedSmesSet = null;

	protected String mbApply = null;
	protected String mbClear = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = appContext.getUserPreferences().getSubjectsFilter();

		if (masks == null)
		{
			checkedSubjectsSet = filter.getNames();
			checkedSmesSet = new HashSet(filter.getSmeIds());
			masks = (String[]) filter.getMaskStrings().toArray(new String[0]);

			checkedSmes = (String[]) checkedSmesSet.toArray(new String[0]);
			checkedSubjects = (String[]) checkedSubjectsSet.toArray(new String[0]);
		}
		if (checkedSubjects == null)
			checkedSubjects = new String[0];
		if (checkedSmes == null)
			checkedSmes = new String[0];

		checkedSubjects = trimStrings(checkedSubjects);
		checkedSmes = trimStrings(checkedSmes);
		try
		{
			masks = MaskList.normalizeMaskList(trimStrings(masks));
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.subjects.incorrectMask, e);
		}

		checkedSubjectsSet = new HashSet(Arrays.asList(checkedSubjects));
		checkedSmesSet = new HashSet(Arrays.asList(checkedSmes));

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			try
			{
				filter.setMasks(masks);
			}
			catch (AdminException e)
			{
				return error(SMSCErrors.error.subjects.masksNotDefined, e);
			}
			filter.setNames(checkedSubjects);
			filter.setSmes(checkedSmes);
			return RESULT_DONE;
		}
		else if (mbClear != null)
		{
			checkedSubjects = checkedSmes = masks = new String[0];

			checkedSubjectsSet.clear();
			checkedSmesSet.clear();
			return RESULT_OK;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		return RESULT_OK;
	}


	public boolean isSubjChecked(String subj)
	{
		return checkedSubjectsSet.contains(subj);
	}

	public boolean isSmeChecked(String sme)
	{
		return checkedSmesSet.contains(sme);
	}

	public Collection getAllSubjects()
	{
		return routeSubjectManager.getSubjects().getNames();
	}

	public Collection getAllSmes()
	{
		return smeManager.getSmeNames();
	}


	/***************************** properties **********************************/
	public String[] getCheckedSubjects()
	{
		return checkedSubjects;
	}

	public void setCheckedSubjects(String[] checkedSubjects)
	{
		this.checkedSubjects = checkedSubjects;
	}

	public String[] getCheckedSmes()
	{
		return checkedSmes;
	}

	public void setCheckedSmes(String[] checkedSmes)
	{
		this.checkedSmes = checkedSmes;
	}

	public String[] getMasks()
	{
		return masks;
	}

	public void setMasks(String[] masks)
	{
		this.masks = masks;
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
