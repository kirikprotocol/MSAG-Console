/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.SubjectQuery;

import java.util.*;

public class Index extends IndexBean
{
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT + 0;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	protected QueryResultSet subjects = null;

	protected String editName = null;

	protected String[] checkedSubjects = new String[0];
	protected Set checkedSubjectsSet = new HashSet();

	protected String mbAdd = null;
	protected String mbDelete = null;
	protected String mbEdit = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = preferences.getSubjectsPageSize();
		if (sort != null)
			preferences.getSubjectsSortOrder().set(0, sort);
		else
			sort = (String) preferences.getSubjectsSortOrder().get(0);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		subjects = new NullResultSet();

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
			result = deleteSubject();

		logger.debug("Subjects.Index - process with sorting [" + (String) preferences.getSubjectsSortOrder().get(0) + "]");
		subjects = routeSubjectManager.getSubjects().query(new SubjectQuery(pageSize, preferences.getSubjectsFilter(), preferences.getSubjectsSortOrder(), startPosition));
		totalSize = subjects.getTotalSize();

		checkedSubjectsSet.addAll(Arrays.asList(checkedSubjects));

		return result;
	}

	protected int deleteSubject()
	{
		int result = RESULT_OK;
		for (int i = 0; i < checkedSubjects.length; i++)
		{
			String subject = checkedSubjects[i];
			if (!routeSubjectManager.getRoutes().isSubjectUsed(subject))
			{
				routeSubjectManager.getSubjects().remove(subject);
				appContext.getStatuses().setSubjectsChanged(true);
			}
			else
				result = error(SMSCErrors.error.subjects.cantDelete, subject);
		}
		checkedSubjects = new String[0];
		checkedSubjectsSet.clear();
		return result;
	}

	public boolean isSubjectChecked(String alias)
	{
		return checkedSubjectsSet.contains(alias);
	}


	/******************** properties *************************/

	public String getEditName()
	{
		return editName;
	}

	public void setEditName(String editName)
	{
		this.editName = editName;
	}

	public String[] getCheckedSubjects()
	{
		return checkedSubjects;
	}

	public void setCheckedSubjects(String[] checkedSubjects)
	{
		this.checkedSubjects = checkedSubjects;
	}

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public QueryResultSet getSubjects()
	{
		return subjects;
	}

	public void setSubjects(QueryResultSet subjects)
	{
		this.subjects = subjects;
	}
}
