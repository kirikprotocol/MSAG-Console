package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.mcisme.backend.Statistics;
import ru.novosoft.smsc.mcisme.backend.StatQuery;
import ru.novosoft.smsc.mcisme.backend.EventsStat;
import ru.novosoft.smsc.mcisme.backend.HourCountersSet;
import ru.novosoft.smsc.mcisme.backend.DateCountersSet;
import ru.novosoft.smsc.mcisme.backend.CountersSet;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.lang.Integer;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 17:47:49
 * To change this template use Options | File Templates.
 */
public class EventsStatistics extends MCISmeBean
{
	private static final String SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
	private static final int SMSC_MCISME_STAT_VERSION_INFO = 0x0001;

    private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

    private Statistics statistics = null;
    private StatQuery query = new StatQuery();
    private EventsStat stat = new EventsStat();

    private String mbQuery = null;

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK) return result;

//      if (getMCISmeContext().getDataSource() != null)
//        stat.setDataSource(getMCISmeContext().getDataSource());

      stat.setMCISme(getMCISmeContext().getMCISme());

      return RESULT_OK;
    }

    public int process(HttpServletRequest request)
    {
		int result = super.process(request);
		if (result != RESULT_OK) return result;
		
		String statPath = null;

		try
		{
			String statDir = getConfig().getString("MCISme.Statistics.statDir");
			statPath = new String(getMCISmeContext().getMCISmeFolder() + '/' + statDir);
			logger.debug(statPath);
		} 
		catch (Exception e) 
		{
			logger.error(e);
			return error("mcisme.error.config_load_failed", e);
		}
		
		if (mbQuery != null)
		{
			try
			{
				statistics = null;
				statistics = stat.getStatistics(statPath, query);
			}
			catch (Throwable e)
			{
				logger.debug("Couldn't get statisctics", e);
				return error("mcisme.error.get_stat", e);
			}
		mbQuery = null;
		}

		return RESULT_OK;
	}

    public Statistics getStatistics() 
	{
      return statistics;
    }

    public String getMbQuery() {
      return mbQuery;
    }
    public void setMbQuery(String mbQuery) {
      this.mbQuery = mbQuery;
    }

    private Date convertStringToDate(String date)
    {
      Date converted = new Date();
      try {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
        converted = formatter.parse(date);
      } catch (ParseException e) {
        e.printStackTrace();
      }
      return converted;
    }

    private String convertDateToString(Date date)
    {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      return formatter.format(date);
    }

    /* -------------------------- StatQuery delegates -------------------------- */
    public void setFromDate(String fromDate)
    {
      if (fromDate != null && fromDate.trim().length() > 0) {
        query.setFromDate(convertStringToDate(fromDate));
        query.setFromDateEnabled(true);
      } else {
        query.setFromDateEnabled(false);
      }
    }
    public String getFromDate() {
      return (query.isFromDateEnabled()) ? convertDateToString(query.getFromDate()) : "";
    }

    public void setTillDate(String tillDate)
    {
      if (tillDate != null && tillDate.trim().length() > 0) {
        query.setTillDate(convertStringToDate(tillDate));
        query.setTillDateEnabled(true);
      } else {
        query.setTillDateEnabled(false);
      }
    }
    public String getTillDate() {
      return (query.isTillDateEnabled()) ? convertDateToString(query.getTillDate()) : "";
    }

}
