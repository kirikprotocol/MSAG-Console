/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasQuery;
import ru.novosoft.smsc.jsp.util.tables.impl.SubjectQuery;

import java.util.List;
import java.util.Arrays;
import java.util.Set;
import java.util.HashSet;

public class Index extends SmscBean
{
	public static final int RESULT_FILTER = PRIVATE_RESULT;
	public static final int RESULT_ADD = PRIVATE_RESULT + 1;
	public static final int RESULT_EDIT = PRIVATE_RESULT + 2;

	protected QueryResultSet subjects = null;

	protected int startPosition = 0;
	protected int totalSize = 0;

	protected String editName = null;

	protected String[] checkedSubjects = new String[0];
	protected Set checkedSubjectsSet = new HashSet();

	protected String mbAdd = null;
	protected String mbDelete = null;
	protected String mbEdit = null;
	protected String mbFilter = null;
	protected String mbFirst = null;
	protected String mbPrev = null;
	protected String mbNext = null;
	protected String mbLast = null;

	protected String sort = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		UserPreferences up = appContext.getUserPreferences();
		int pageSize = up.getSubjectsPageSize();
		if (sort != null)
			up.getSubjectsSortOrder().set(0, sort);
		else
			sort = (String) up.getSubjectsSortOrder().get(0);

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbFilter != null)
			return RESULT_FILTER;
		else if (mbDelete != null)
		{
			int dresult = deleteSubject();
			if (dresult != RESULT_OK)
				return result;
		}
		else if (mbFirst != null)
			startPosition = 0;
		else if (mbPrev != null)
			startPosition = startPosition > pageSize ? startPosition - pageSize : 0;
		else if (mbNext != null)
			startPosition += pageSize;
		else if (mbLast != null)
			startPosition = (totalSize / pageSize + (totalSize % pageSize == 0 ? -1 : 0)) * pageSize;

		logger.debug("Subjects.Index - process with sorting [" + (String) up.getSubjectsSortOrder().get(0) + "]");
		subjects = smsc.getSubjects().query(new SubjectQuery(pageSize, up.getSubjectsFilter(), up.getSubjectsSortOrder(), startPosition));
		totalSize = subjects.getTotalSize();

		checkedSubjectsSet.addAll(Arrays.asList(checkedSubjects));

		return RESULT_OK;
	}

	protected int deleteSubject()
	{
		for (int i = 0; i < checkedSubjects.length; i++)
		{
			String subject = checkedSubjects[i];
			smsc.getSubjects().remove(subject);
		}
		checkedSubjects = new String[0];
		checkedSubjectsSet.clear();
		return RESULT_OK;
	}

	public boolean isFirst()
	{
		return startPosition == 0;
	}

	public boolean isLast()
	{
		return subjects.isLast();
	}

	public boolean isSubjectChecked(String alias)
	{
		return checkedSubjectsSet.contains(alias);
	}


	/******************** properties *************************/

	public int getStartPosition()
	{
		return startPosition;
	}

	public void setStartPosition(int startPosition)
	{
		this.startPosition = startPosition;
	}

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

	public String getMbFilter()
	{
		return mbFilter;
	}

	public void setMbFilter(String mbFilter)
	{
		this.mbFilter = mbFilter;
	}

	public String getMbFirst()
	{
		return mbFirst;
	}

	public void setMbFirst(String mbFirst)
	{
		this.mbFirst = mbFirst;
	}

	public String getMbPrev()
	{
		return mbPrev;
	}

	public void setMbPrev(String mbPrev)
	{
		this.mbPrev = mbPrev;
	}

	public String getMbNext()
	{
		return mbNext;
	}

	public void setMbNext(String mbNext)
	{
		this.mbNext = mbNext;
	}

	public String getMbLast()
	{
		return mbLast;
	}

	public void setMbLast(String mbLast)
	{
		this.mbLast = mbLast;
	}

	public String getSort()
	{
		return sort;
	}

	public void setSort(String sort)
	{
		this.sort = sort;
	}

	public int getTotalSize()
	{
		return totalSize;
	}

	public void setTotalSize(int totalSize)
	{
		this.totalSize = totalSize;
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
