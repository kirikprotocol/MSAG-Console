/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Index extends IndexBean {
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	private QueryResultSet profiles = null;

	private String profileMask = null;
	private String[] masks = new String[0];
	private boolean initialized = false;
	private String[] checked = new String[0];
	private Set checkedSet = new HashSet();

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

		if (masks == null)
			masks = new String[0];
		masks = trimStrings(masks);
		if (sort == null)
			sort = (String) preferences.getProfilesSortOrder().get(0);
		else
			preferences.getProfilesSortOrder().set(0, sort);

		if (!initialized) {
			masks = (String[]) preferences.getProfilesFilter().getMasks().getNames().toArray(new String[0]);
		} else {
			try {
				preferences.getProfilesFilter().setMasks(new MaskList(masks));
			} catch (AdminException e) {
				logger.debug("Couldn't create mask list: ", e);
			}
		}

		if (checked == null)
			checked = new String[0];
		checkedSet.addAll(Arrays.asList(checked));

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		profiles = new NullResultSet();
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

		if (masks != null && masks.length > 0) {
			try {
				logger.debug("Profiles.Index - process with sorting [" + (String) preferences.getProfilesSortOrder().get(0) + "]");
				profiles = smsc.profilesQuery(new ProfileQuery(pageSize, preferences.getProfilesFilter(), preferences.getProfilesSortOrder(), startPosition, ProfileQuery.SHOW_ADDRESSES));
				totalSize = profiles.getTotalSize();
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

	public String[] getMasks()
	{
		return masks;
	}

	public void setMasks(String[] masks)
	{
		this.masks = masks;
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
}
