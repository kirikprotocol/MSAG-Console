package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;

/**
 * Created by igork
 * Date: 26.02.2003
 * Time: 18:36:25
 */
public class dlBody extends IndexBean
{
	protected String name = null;
	protected String owner = null;
	protected int maxElements = 0;
	private String[] submitters = null;
	private String[] members = null; //receivers
	protected List fullMembersList = null;
	protected List fullSubmittersList = null;
	private Set oldMembers = null;
	private Set oldSubmitters = null;
	protected boolean initialized = false;

	private void processAddresses(List fullList, Set oldSet, String[] newList) throws AdminException
	{
		MaskList tmp = new MaskList(trimStrings(newList));
		HashSet newSet = new HashSet(tmp.getNames());
		Set intersection = new HashSet(newSet);
		intersection.retainAll(oldSet);
		Set toDel = new HashSet(oldSet);
		toDel.removeAll(newSet);
		Set toAdd = new HashSet(newSet);
		toAdd.removeAll(oldSet);
		fullList.removeAll(toDel);
		fullList.addAll(toAdd);
		oldSet.clear();
		oldSet.addAll(newSet);
	}

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (name == null)
		{
			name = "";
			owner = "";
			maxElements = 0;
		}
		if (submitters == null)
			submitters = new String[0];
		if (members == null)
			members = new String[0];

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		try
		{
			if ((result = processSubmitters()) != RESULT_OK)
				return result;
			if ((result = processMembers()) != RESULT_OK)
				return result;
		}
		catch (AdminException e)
		{
			logger.error("Errors in entered data", e);
			return error(SMSCErrors.error.dl.invalidAddress, e);
		}

		return RESULT_OK;
	}

	private int processSubmitters() throws AdminException
	{
		if (oldSubmitters != null)
			processAddresses(fullSubmittersList, oldSubmitters, submitters);
		else
			oldSubmitters = new HashSet(fullSubmittersList);

		submitters = (String[]) new SortedList(fullSubmittersList).toArray(new String[0]);

		return RESULT_OK;
	}

	private int processMembers() throws AdminException
	{
		if (pageSize == 0)
			pageSize = preferences.getDlPageSize();
		else
			preferences.setDlPageSize(pageSize);

		if (sort == null || sort.length() == 0)
			sort = preferences.getDlSortOrder();
		else
			preferences.setDlSortOrder(sort);

		sort(fullMembersList, sort);

		if (oldMembers != null)
			processAddresses(fullMembersList, oldMembers, members);

		totalSize = fullMembersList.size();
		List newMembersList = fullMembersList.subList(Math.max(startPosition, 0), Math.min(startPosition + pageSize, fullMembersList.size()));
		members = (String[]) newMembersList.toArray(new String[0]);
		if (oldMembers == null)
		{
			oldMembers = new HashSet(newMembersList);
		}
		else
		{
			oldMembers.clear();
			oldMembers.addAll(newMembersList);
		}
		return RESULT_OK;
	}

	private static void sort(List listToSort, String sortOption)
	{
		//sort members
		final boolean isNegativeSort = sortOption.startsWith("-");
		Collections.sort(listToSort, new Comparator()
		{
			public int compare(Object o1, Object o2)
			{
				String s1 = (String) o1;
				String s2 = (String) o2;
				return isNegativeSort ? s2.compareTo(s1) : s1.compareTo(s2);
			}
		});
	}


	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	public String getOwner()
	{
		return owner;
	}

	public void setOwner(String owner)
	{
		this.owner = owner;
	}

	public int getMaxElements()
	{
		return maxElements;
	}

	public void setMaxElements(int maxElements)
	{
		this.maxElements = maxElements;
	}

	public String[] getSubmitters()
	{
		return submitters;
	}

	public void setSubmitters(String[] submitters)
	{
		this.submitters = submitters;
	}

	public String[] getMembers()
	{
		return members;
	}

	public void setMembers(String[] members)
	{
		this.members = members;
	}

	protected void clear()
	{
		super.clear();
		name = owner = null;
		submitters = members = null;
		fullMembersList = fullSubmittersList = null;
		oldMembers = oldSubmitters = null;
		maxElements = 0;
		initialized = false;
	}
}
