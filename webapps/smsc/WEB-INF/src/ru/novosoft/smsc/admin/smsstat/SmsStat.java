/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:41:41 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.smsc_service.Smsc;

import java.util.Date;

import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.text.SimpleDateFormat;
import java.text.ParseException;

public class SmsStat
{
    private static final String SMS_QUERY =     // group by period
        "SELECT period, sum(received), sum(finalized), sum(rescheduled) FROM sms_stat_sms ";
    private static final String SME_QUERY =     // group by systemid
        "SELECT systemid, sum(received), sum(sent) FROM sms_stat_sme ";
    private static final String STATE_QUERY =   // group by errcode
        "SELECT errcode, sum(counter) FROM sms_stat_state ";
    private static final String ROUTE_QUERY =   // group by routeid
        "SELECT routeid, sum(processed) FROM sms_stat_route ";

    private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";

    private DataSource ds = null;
    private Smsc smsc = null;

    private Statistics stat = null;

    public void setDataSource(DataSource ds) { this.ds = ds; }
    public void setSmsc(Smsc smsc) { this.smsc = smsc; }

    private String preparePeriodPart(StatQuery query) {
        String str = (query.isFromDateEnabled() ||
                      query.isTillDateEnabled()) ? " WHERE ":"";
        if (query.isFromDateEnabled()) {
            str += " period >= ? ";
            if (query.isTillDateEnabled()) str += " AND ";
        }
        if (query.isTillDateEnabled()) str += " period <= ? ";
        return str;
    }
    private String prepareSmsQuery(StatQuery query) {
        return SMS_QUERY+preparePeriodPart(query)+"GROUP BY period ORDER BY period ASC";
    }
    private String prepareSmeQuery(StatQuery query) {
        return SME_QUERY+preparePeriodPart(query)+"GROUP BY systemid";
    }
    private String prepareStateQuery(StatQuery query) {
        return STATE_QUERY+preparePeriodPart(query)+"GROUP BY errcode";
    }
    private String prepareRouteQuery(StatQuery query) {
        return ROUTE_QUERY+preparePeriodPart(query)+"GROUP BY routeid";
    }
    private void bindPeriodPart(PreparedStatement stmt, StatQuery query)
        throws SQLException
    {
        int pos=1;
        if (query.isFromDateEnabled())
          stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
        if (query.isTillDateEnabled())
          stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
    }

    public Statistics getStatistics(StatQuery query)
    {
        Connection connection = null;
        stat = new Statistics();
        try
        {
            connection = ds.getConnection();
            if (connection == null) return stat;

            processSmsQuery(connection, query);
            processSmeQuery(connection, query);
            processStateQuery(connection, query);
            processRouteQuery(connection, query);

            connection.close();
        }
        catch (Exception exc)
        {
          try { if (connection != null) connection.close(); }
          catch (Exception cexc) { cexc.printStackTrace(); }
          System.out.println("Operation with DB failed !");
          exc.printStackTrace();
        }
        return stat;
    }

    private ResultSet processQuery(Connection connection, StatQuery query, String sql)
        throws SQLException
    {
        PreparedStatement stmt = connection.prepareStatement(sql);
        bindPeriodPart(stmt, query);
        return stmt.executeQuery();
    }
    private void processSmsQuery(Connection connection, StatQuery query)
        throws SQLException
    {
        int oldPeriod = 0;
        DateCountersSet dateCounters = null;
        ResultSet rs = processQuery(connection, query, prepareSmsQuery(query));

        while (rs.next())
        {
            int newPeriod = rs.getInt(1);
            int hour = calculateHour(newPeriod);
            HourCountersSet hourCounters = new HourCountersSet(
                    rs.getInt(2), rs.getInt(3), rs.getInt(4), hour);
            if (dateCounters == null) { // on first iteration
                Date date = calculateDate(newPeriod);
                dateCounters = new DateCountersSet(date);
            }
            else if (needChangeDate(oldPeriod, newPeriod)) { // on date changed
                stat.addDateStat(dateCounters);
                Date date = calculateDate(newPeriod);
                dateCounters = new DateCountersSet(date);
            }
            dateCounters.addHourStat(hourCounters);
            oldPeriod = newPeriod;
        }

        if (dateCounters != null) {
            stat.addDateStat(dateCounters);
        }
    }
    private void processSmeQuery(Connection connection, StatQuery query)
        throws SQLException
    {
        ResultSet rs = processQuery(connection, query, prepareSmeQuery(query));
        while (rs.next()) {
            stat.addSmeIdStat(new SmeIdCountersSet(
                    rs.getString(1), rs.getInt(2), rs.getInt(3)));
        }
    }
    private void processStateQuery(Connection connection, StatQuery query)
        throws SQLException
    {
        ResultSet rs = processQuery(connection, query, prepareStateQuery(query));
        while (rs.next()) {
            stat.addErrorStat(new ErrorCounterSet(
                    rs.getInt(1), rs.getInt(2)));
        }
    }
    private void processRouteQuery(Connection connection, StatQuery query)
        throws SQLException
    {
        ResultSet rs = processQuery(connection, query, prepareRouteQuery(query));
        while (rs.next()) {
            stat.addRouteIdStat(new RouteIdCountersSet(
                    rs.getString(1), rs.getInt(2)));
        }
    }

    private boolean needChangeDate(int oldPeriod, int newPeriod) {
        String oldStr = Integer.toString(oldPeriod);
        String newStr = Integer.toString(newPeriod);
        oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length()-2));
        newPeriod = Integer.parseInt(newStr.substring(0, newStr.length()-2));
        return (oldPeriod != newPeriod);
    }
    private int calculatePeriod(Date date) {
        SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
        return Integer.parseInt(formatter.format(date));
    }
    private Date calculateDate(int period) {
        Date converted = new Date();
        String str = Integer.toString(period);
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
            converted = formatter.parse(str);
        }
        catch (ParseException e) {
            e.printStackTrace();
        }
        return converted;
    }
    private int calculateHour(int period) {
        String str = Integer.toString(period);
        return Integer.parseInt(str.substring(str.length()-2));
    }
}
