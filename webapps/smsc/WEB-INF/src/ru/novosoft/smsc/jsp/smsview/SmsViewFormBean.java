package ru.novosoft.smsc.jsp.smsview;

import ru.novosoft.smsc.admin.smsview.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

public class SmsViewFormBean extends IndexBean
{
	private SmsQuery query = new SmsQuery();
	private SmsSet rows = null;
	private SmsView view = new SmsView();
	private int deletedRowsCount = 0;

	public static String monthesNames[] = {
		"Jan", "Feb", "Mar", "Apr",
		"May", "Jun", "Jul", "Aug", "Sep",
		"Oct", "Nov", "Dec"
	};

	private String mbDelete = null;
	private String mbQuery = null;
    private String mbClear = null;
	private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
	private String oldSort = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (sort != null)
			preferences.setSmsviewSortOrder(sort);
		else
			sort = preferences.getSmsviewSortOrder();

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		if (this.appContext == null && appContext instanceof SMSCAppContext)
		{
			view.setDataSource(appContext.getConnectionPool());
			view.setSmsc(appContext.getSmsc());
		}

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbDelete != null)
			result = processDeleteAll();
		else if (mbQuery != null)
			result = processQuery();
        else if (mbClear != null)
            result = clearQuery();
		else
			result = processResortAndNavigate(false);

		mbDelete = null;
		mbQuery = null;
        mbClear = null;

		return result;
	}

	private int processResortAndNavigate(boolean force)
	{
		if (force || (sort != null && sort.length() > 0 && !sort.equals(oldSort)))
		{
			final boolean isNegativeSort = sort.startsWith("-");
			final String sortField = isNegativeSort ? sort.substring(1) : sort;

			if (rows != null && rows.getRowsList() != null)
			{
				Collections.sort(rows.getRowsList(), new Comparator()
				{
					public int compare(Object o1, Object o2)
					{
						int result = 0;
						SmsRow r1 = (SmsRow) o1;
						SmsRow r2 = (SmsRow) o2;
						if (sortField.equalsIgnoreCase("name"))
							result = r1.getIdString().compareTo(r2.getIdString());
						if (sortField.equalsIgnoreCase("date"))
							result = r1.getDate().compareTo(r2.getDate());
						if (sortField.equalsIgnoreCase("from"))
							result = r1.getFrom().compareTo(r2.getFrom());
						if (sortField.equalsIgnoreCase("to"))
							result = r1.getTo().compareTo(r2.getTo());
						if (sortField.equalsIgnoreCase("status"))
							result = r1.getStatus().compareTo(r2.getStatus());

						return isNegativeSort ? -result : result;
					}
				});
			}
			oldSort = sort;
			startPosition = 0;
		}
		return RESULT_OK;
	}

	public int processQuery()
	{
		rows = view.getSmsSet(query);
		startPosition = 0;
		totalSize = rows.getRowsCount();
		processResortAndNavigate(true);
		return RESULT_OK;
	}
    public int clearQuery()
    {
        rows = null; startPosition = 0;
        totalSize = 0;
        query = new SmsQuery();
        processResortAndNavigate(true);
        return RESULT_OK;
    }

	public int processDeleteAll()
	{
		if (getStorageType() == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE)
			deletedRowsCount = view.delArchiveSmsSet(rows);
		else
			deletedRowsCount = view.delOperativeSmsSet(rows);

		return processQuery();
	}

	public SmsRow getRow(int index)	{
        return rows == null ? null : rows.getRow(index);
	}

	/********************************* query delegeates *********************************/
    public void refreshQuery() {
        query = new SmsQuery();
        setFromDate(null);
        setTillDate(null);
    }
	public void setSort(String by) {
		sort = by;
	}
	public String getSort() {
		return sort;
	}
	public int getStorageType() {
		return query.getStorageType();
	}
	public void setStorageType(int type) {
		query.setStorageType(type);
	}
	public int getRowsMaximum() {
		return query.getRowsMaximum();
	}
	public void setRowsMaximum(int max) {
		query.setRowsMaximum(max);
	}
	public String getFromAddress() {
		return query.getFromAddress();
	}
	public void setFromAddress(String address) {
		query.setFromAddress(address);
	}
	public String getToAddress() {
		return query.getToAddress();
	}
	public void setToAddress(String address) {
		query.setToAddress(address);
	}
	public String getSrcSmeId()	{
		return query.getSrcSmeId();
	}
	public void setSrcSmeId(String id) {
		query.setSrcSmeId(id);
	}
	public String getDstSmeId() {
		return query.getDstSmeId();
	}
	public void setDstSmeId(String id)	{
		query.setDstSmeId(id);
	}
	public String getRouteId() {
		return query.getRouteId();
	}
	public void setRouteId(String id) {
		query.setRouteId(id);
	}
	public String getSmsId() {
		return query.getSmsId();
	}
	public void setSmsId(String id)	{
		query.setSmsId(id);
	}

    public String getFromDate()
    {
		if (query.getFromDateEnabled())
		{
			SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            return formatter.format(query.getFromDate());
		}
		else
		{
			return "";
		}
	}

    public void setFromDate(String dateString)
	{
		final boolean dateEnabled = dateString != null && dateString.trim().length() > 0;
		query.setFromDateEnabled(dateEnabled);
		if (dateEnabled)
		{
			try
			{
				SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
                query.setFromDate(formatter.parse(dateString));
			}
			catch (ParseException e)
			{
				query.setFromDate(new Date());
				e.printStackTrace();
			}
		}
	}

	public String getTillDate()
	{
		if (query.getTillDateEnabled())
		{
			SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            return formatter.format(query.getTillDate());
		}
		else
		{
			return "";
		}
	}

	public void setTillDate(String dateString)
	{
		final boolean dateEnabled = dateString != null && dateString.trim().length() > 0;
		query.setTillDateEnabled(dateEnabled);
		if (dateEnabled)
		{
			try
			{
				SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
                query.setTillDate(formatter.parse(dateString));
			}
			catch (ParseException e)
			{
				query.setTillDate(new Date());
				e.printStackTrace();
			}
		}
	}

	public String getMbDelete() { return mbDelete; }
	public void setMbDelete(String mbDelete) {	this.mbDelete = mbDelete; }
	public String getMbQuery() { return mbQuery; }
	public void setMbQuery(String mbQuery) { this.mbQuery = mbQuery; }
    public String getMbClear() { return mbClear; }
    public void setMbClear(String mbClear) { this.mbClear = mbClear; }
}