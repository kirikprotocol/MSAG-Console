package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 11, 2003
 * Time: 1:54:30 PM
 */
public class Providers extends DbsmeBean {
	public static final int RESULT_EDIT = DbsmeBean.PRIVATE_RESULT;
	public static final int RESULT_ADD = DbsmeBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = DbsmeBean.PRIVATE_RESULT + 2;

	public static final String prefix = "DBSme.DataProviders";
	public static final int prefix_length = prefix.length();

	private String mbEdit = null;

	private String providerName = null;
	private QueryResultSet providers = null;
	private String[] checked = new String[0];
	private Set checkedSet = new HashSet();
	private String mbAdd = null;
	private String mbDelete = null;

	private class _DataItem extends AbstractDataItem {
		protected _DataItem(String name, String address)
		{
			values.put("name", name);
			values.put("address", address);
		}
	}

	private class _DataSource extends AbstractDataSourceImpl {

		public _DataSource()
		{
			super(new String[]{"name", "address"});
		}

		public QueryResultSet query(_Query query_to_run)
		{
			clear();
			for (Iterator i = getSectionChildSectionNames(prefix).iterator(); i.hasNext();) {
				String fullProviderName = (String) i.next();
				String dpName = StringEncoderDecoder.decodeDot(fullProviderName.substring(prefix_length + 1));
				String dpAddress = "";
				try {
					dpAddress = getString(fullProviderName + ".address");
				} catch (Exception e) {
					logger.debug("Couldn't get provider address, nested: " + e.getMessage(), e);
				}
				add(new _DataItem(dpName, dpAddress));
			}
			return super.query(query_to_run);
		}
	}

	private class _Query extends AbstractQueryImpl {
		public _Query(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition)
		{
			super(expectedResultsQuantity, filter, sortOrder, startPosition);
		}
	}

	private class _Filter implements Filter {
		public boolean isEmpty()
		{
			return true;
		}

		public boolean isItemAllowed(DataItem item)
		{
			return true;
		}
	}


	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = 20;
		if (sort != null) {
			//preferences.getAliasesSortOrder().set(0, sort);
		} else {
			//sort = (String) preferences.getAliasesSortOrder().get(0);
			sort = "name";
		}
		return result;
	}

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbEdit != null && mbEdit.length() > 0 && providerName != null && providerName.length() > 0)
			return RESULT_EDIT;
		if (mbAdd != null)
			return RESULT_ADD;
		if (mbDelete != null)
			return delete();

		Vector sortVector = new Vector();
		sortVector.add(sort);
		providers = new _DataSource().query(new _Query(pageSize, new _Filter(), sortVector, startPosition));
		totalSize = providers.getTotalSize();

		checkedSet.addAll(Arrays.asList(checked));

		return result;
	}

	private int delete()
	{
		for (int i = 0; i < checked.length; i++)
			config.removeSection("DBSme.DataProviders." + StringEncoderDecoder.encodeDot(checked[i]));

		try {
			config.save();
		} catch (Exception e) {
			logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
			return error(DBSmeErrors.error.couldntSaveTempConfig, e);
		}

		DbSmeContext.getInstance(appContext).setConfigChanged(true);

		checkedSet.clear();
		checked = new String[0];
		return RESULT_DONE;
	}

	public boolean isDpChecked(String dpName)
	{
		return checkedSet.contains(dpName);
	}

	public String[] getChecked()
	{
		return checked;
	}

	public void setChecked(String[] checked)
	{
		this.checked = checked;
	}

	public QueryResultSet getProviders()
	{
		return providers;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public String getProviderName()
	{
		return providerName;
	}

	public void setProviderName(String providerName)
	{
		this.providerName = providerName;
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
}
