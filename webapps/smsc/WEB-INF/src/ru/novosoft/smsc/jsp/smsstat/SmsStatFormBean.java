/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:38:53 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.smsstat;

import java.util.Date;
import java.util.List;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import ru.novosoft.smsc.admin.smsstat.StatQuery;
import ru.novosoft.smsc.admin.smsstat.SmsStat;
import ru.novosoft.smsc.admin.smsstat.Statistics;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;


public class SmsStatFormBean extends IndexBean
{
    private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

    private Statistics statistics = null;
    private StatQuery query = new StatQuery();
    private SmsStat stat = new SmsStat();

    private String mbQuery = null;

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        return RESULT_OK;
    }

    public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
    {
        if (this.appContext == null && appContext instanceof SMSCAppContext)
        {
            stat.setDataSource(appContext.getConnectionPool());
            stat.setSmsc(appContext.getSmsc());
        }

        if (mbQuery != null)
            statistics = stat.getStatistics(query);

        mbQuery = null;
        return RESULT_OK;
    }

    public Statistics getStatistics() {
        return statistics;
    }
    public String getMbQuery() {
        return mbQuery;
    }
    public void setMbQuery(String mbQuery) {
        this.mbQuery = mbQuery;
    }

    private Date convertStringToDate(String date) {
        Date converted = new Date();
        try
        {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            converted = formatter.parse(date);
        }
        catch (ParseException e)
        {
            e.printStackTrace();
        }
        return converted;
    }
    private String convertDateToString(Date date) {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
        return formatter.format(date);
    }

    /* -------------------------- StatQuery delegates -------------------------- */
    public void setFromDate(String fromDate) {
        if (fromDate != null && fromDate.trim().length() >0) {
            query.setFromDate(convertStringToDate(fromDate));
            query.setFromDateEnabled(true);
        } else {
            query.setFromDateEnabled(false);
        }
    }
    public String getFromDate() {
        return (query.isFromDateEnabled()) ?
                convertDateToString(query.getFromDate()) : "";
    }
    public void setTillDate(String tillDate) {
        if (tillDate != null && tillDate.trim().length() >0) {
            query.setTillDate(convertStringToDate(tillDate));
            query.setTillDateEnabled(true);
        } else {
            query.setTillDateEnabled(false);
        }
    }
    public String getTillDate() {
        return (query.isTillDateEnabled()) ?
                convertDateToString(query.getTillDate()) : "";
    }

}
