/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 3:06:00 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.*;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;

import java.util.*;

public class DistributionListAdminFormBean extends IndexBean
{
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT + 1;
	protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	private DistributionListAdmin admin = null;
	private List dls = null;
	private List dlNames = null;
	private String[] checkedDls = new String[0];
	private Set checkedDlsSet = null;
	private String editDl = null;

	private String mbEdit = null;
	private String mbDelete = null;
	private String mbAdd = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		//?
		if (this.admin == null)
		{
			admin = appContext.getSmsc().getDistributionListAdmin();
		}

		if (pageSize == 0)
			pageSize = 20;
		if (sort == null || sort.length() == 0)
			sort = "name";

		if (checkedDls == null)
			checkedDls = new String[0];
		checkedDlsSet = new HashSet(Arrays.asList(checkedDls));
		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		dlNames = new ArrayList();
		//query names
		try
		{
			dls = admin.list(appContext.getUserPreferences().getDlFilter());
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.dl.CouldntListDistributionLists);
		}

		totalSize = dls.size();

		for (Iterator i = dls.iterator(); i.hasNext();)
			dlNames.add(((DistributionList) i.next()).getName());
		Collections.sort(dlNames);
		dlNames = dlNames.subList(startPosition, Math.min(startPosition + pageSize, dlNames.size()));
		if (sort.startsWith("-"))
			Collections.reverse(dlNames);

		if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
			return delete();
		else if (mbAdd != null)
			return RESULT_ADD;

		return result;
	}

	private int delete()
	{
		int result = RESULT_OK;
		for (int i = 0; i < checkedDls.length; i++)
		{
			String checkedDl = checkedDls[i];
			try
			{
				admin.deleteDistributionList(checkedDl);
				checkedDlsSet.remove(checkedDl);
				dlNames.remove(checkedDl);
				logger.debug("Distribution list \"" + checkedDl + "\" deleted");
			}
			catch (ListNotExistsException e)
			{
				logger.error("Couldn't delete distribution list \"" + checkedDl + "\": unknown DL");
				result = error(SMSCErrors.error.dl.unknownDL, checkedDl);
			}
			catch (AdminException e)
			{
				logger.error("Couldn't delete distribution list \"" + checkedDl + '"');
				result = error(SMSCErrors.error.dl.couldntdelete, checkedDl);
			}
		}
		return result;
	}

	public boolean isDlChecked(String name)
	{
		return checkedDlsSet.contains(name);
	}

	public List getDlNames()
	{
		return dlNames;
	}

	public void setDlNames(List dlNames)
	{
		this.dlNames = dlNames;
	}

	public String[] getCheckedDls()
	{
		return checkedDls;
	}

	public void setCheckedDls(String[] checkedDls)
	{
		this.checkedDls = checkedDls;
	}

	public String getEditDl()
	{
		return editDl;
	}

	public void setEditDl(String editDl)
	{
		this.editDl = editDl;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}
}
