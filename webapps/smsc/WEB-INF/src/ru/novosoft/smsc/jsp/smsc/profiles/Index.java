/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;

import java.util.*;

public class Index extends IndexBean {
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int RESULT_REFRESH = IndexBean.PRIVATE_RESULT + 2;
	protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 3;

	private QueryResultSet profiles = null;
	private boolean queried = false;

	private String profileMask = null;
	private String filterMask = null;
	private boolean initialized = false;
	private String[] checked = new String[0];
	private Set checkedSet = new HashSet();
	private boolean refreshed = false;

	private String mbAdd = null;
	private String mbEdit = null;
	private String mbDelete = null;
	private String mbAddMask = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = preferences.getProfilesPageSize();

		if (sort == null)
			sort = preferences.getProfilesSortOrder();
		else
			preferences.setProfilesSortOrder(sort);

		if (!initialized) {
			filterMask = preferences.getProfilesFilter();
			if (filterMask == null || filterMask.length() == 0)
				filterMask = "*";
		}
		else
			preferences.setProfilesFilter(filterMask);

		if (checked == null)
			checked = new String[0];
		checkedSet.addAll(Arrays.asList(checked));

		return RESULT_OK;
	}

	private static String normalizeAddresPrefix(String prefix) throws AdminException
	{
		if (prefix == null || prefix.length() == 0)
			return "";
		if (prefix.equals("*"))
			return prefix;

		if (prefix.startsWith("."))
		{
			int dp = prefix.indexOf('.', 1);
			int dp2 = prefix.indexOf('.', dp + 1);
			if (dp < 0 || dp2 < 0)
				throw new AdminException("Mask \"" + prefix + "\" is not valid");

			return prefix;
		}
		else if (prefix.startsWith("+"))
		{
			return ".1.1." + prefix.substring(1);
		}
		else
		{
			return ".0.1." + prefix;
		}
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		profiles = new EmptyResultSet();
		totalSize = 0;

		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
			return delete();
		else if (mbAddMask != null)
			return RESULT_DONE;
		else if (!refreshed)
			return RESULT_REFRESH;

		if (initialized && preferences.getProfilesFilter() != null && preferences.getProfilesFilter().length() > 0) {
			try {
				profiles = smsc.profilesQuery(new ProfileQuery(pageSize, normalizeAddresPrefix(preferences.getProfilesFilter()), preferences.getProfilesSortOrder(), startPosition, ProfileQuery.SHOW_ADDRESSES));
				totalSize = profiles.getTotalSize();
				queried = true;
			} catch (AdminException e) {
				logger.error("Couldn't query profiles", e);
				return error(SMSCErrors.error.profiles.queryError, e);
			}
		}

		return isEditAllowed() ? RESULT_OK : error(SMSCErrors.error.profiles.smscNotConnected);
	}

	private int delete()
	{
		int result = RESULT_DONE;
		for (int i = 0; i < checked.length; i++) {
			String profileMask = checked[i];
			try {
				smsc.profileDelete(new Mask(profileMask));
			} catch (AdminException e) {
				logger.debug("Couldn't delete profile \"" + profileMask + '"', e);
				result = error(SMSCErrors.error.profiles.couldntDelete, profileMask, e);
			}
		}
		return result;
	}

	public QueryResultSet getProfiles()
	{
		return profiles;
	}

	/******************** properties *************************/

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public boolean isEditAllowed()
	{
		try {
			return hostsManager.getServiceInfo(Constants.SMSC_SME_ID).getStatus() == ServiceInfo.STATUS_RUNNING;
		} catch (AdminException e) {
			logger.debug("Couldn't get SMSC service status", e);
		}
		return false;
	}

	public boolean isInitialized()
	{
		return initialized;
	}

	public void setInitialized(boolean initialized)
	{
		this.initialized = initialized;
	}

	public String getProfileMask()
	{
		return profileMask;
	}

	public void setProfileMask(String profileMask)
	{
		this.profileMask = profileMask;
	}

	public boolean isProfileCheked(String profileMask)
	{
		return checkedSet.contains(profileMask);
	}

	public String[] getChecked()
	{
		return checked;
	}

	public void setChecked(String[] checked)
	{
		this.checked = checked;
	}

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String getMbAddMask()
	{
		return mbAddMask;
	}

	public void setMbAddMask(String mbAddMask)
	{
		this.mbAddMask = mbAddMask;
	}

	public boolean isQueried()
	{
		return queried;
	}

	public String getFilterMask()
	{
		return filterMask == null ? "" : filterMask;
	}

	public void setFilterMask(String filterMask)
	{
		this.filterMask = filterMask;
	}

	public boolean isRefreshed()
	{
		return refreshed;
	}

	public void setRefreshed(boolean refreshed)
	{
		this.refreshed = refreshed;
	}
}
