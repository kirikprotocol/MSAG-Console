package ru.novosoft.smsc.jsp.smsview;

import ru.novosoft.smsc.admin.smsview.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

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
	private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

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
		else
			result = processResortAndNavigate();

		mbDelete = null;
		mbQuery = null;

		return result;
	}

	private int processResortAndNavigate()
	{
		return 0;
	}

	public int processQuery()
	{
		rows = view.getSmsSet(query);
		startPosition = 0;
		totalSize = rows.getRowsCount();
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

	public SmsRow getRow(int index)
	{
		return rows == null ? null : rows.getRow(index);
	}

	/********************************* query delegeates *********************************/
	public void setSort(String by)
	{
		query.setSortBy(by);
	}

	public String getSort()
	{
		return query.getSortBy();
	}

	public int getStorageType()
	{
		return query.getStorageType();
	}

	public void setStorageType(int type)
	{
		query.setStorageType(type);
	}

	public int getRowsMaximum()
	{
		return query.getRowsMaximum();
	}

	public void setRowsMaximum(int max)
	{
		query.setRowsMaximum(max);
	}

	public String getFromAddress()
	{
		return query.getFromAddress();
	}

	public void setFromAddress(String address)
	{
		query.setFromAddress(address);
	}

	public String getToAddress()
	{
		return query.getToAddress();
	}

	public void setToAddress(String address)
	{
		query.setToAddress(address);
	}

	public String getSrcSmeId()
	{
		return query.getSrcSmeId();
	}

	public void setSrcSmeId(String id)
	{
		query.setSrcSmeId(id);
	}

	public String getDstSmeId()
	{
		return query.getDstSmeId();
	}

	public void setDstSmeId(String id)
	{
		query.setDstSmeId(id);
	}

	public String getRouteId()
	{
		return query.getRouteId();
	}

	public void setRouteId(String id)
	{
		query.setRouteId(id);
	}

	public String getSmsId()
	{
		return query.getSmsId();
	}

	public void setSmsId(String id)
	{
		query.setSmsId(id);
	}

	public void setFromDateEnabled(boolean enabled)
	{
		query.setFromDateEnabled(enabled);
	}

	public boolean getFromDateEnabled()
	{
		return query.getFromDateEnabled();
	}

	public String getFromDate()
	{
		SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
		return formatter.format(query.getFromDate());
	}

	public void setFromDate(String dateString)
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

	public void setTillDateEnabled(boolean enabled)
	{
		query.setTillDateEnabled(enabled);
	}

	public boolean getTillDateEnabled()
	{
		return query.getTillDateEnabled();
	}

	public String getTillDate()
	{
		SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
		return formatter.format(query.getTillDate());
	}

	public void setTillDate(String dateString)
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

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String getMbQuery()
	{
		return mbQuery;
	}

	public void setMbQuery(String mbQuery)
	{
		this.mbQuery = mbQuery;
	}
}