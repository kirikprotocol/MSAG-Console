/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:41:41 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.AdminException;

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
  private final static String ERRORS_SET =
    " errcode, sum(counter) ";
  private final static String VALUES_SET =
    " sum(accepted), sum(rejected), sum(delivered), sum(failed),"+
    " sum(rescheduled), sum(temporal), max(peak_i), max(peak_o) ";

  private final static String SMS_QUERY =   // group by period
    "SELECT period,"+VALUES_SET+"FROM sms_stat_sms ";
  private final static String SME_QUERY =  // group by systemid
    "SELECT systemid,"+VALUES_SET+"FROM sms_stat_sme ";
  private static final String ROUTE_QUERY =   // group by routeid
    "SELECT routeid,"+VALUES_SET+"FROM sms_stat_route ";

  private static final String STATE_QUERY =       // group by errcode
    "SELECT"+ERRORS_SET+"FROM sms_stat_state ";
  private static final String SME_STATE_QUERY =   // group by errcode
    "SELECT"+ERRORS_SET+"FROM sms_stat_sme_state ";
  private static final String ROUTE_STATE_QUERY = // group by errcode
    "SELECT"+ERRORS_SET+"FROM sms_stat_route_state ";

  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";

  private DataSource ds = null;
  private Smsc smsc = null;

  private Statistics stat = null;

  public void setDataSource(DataSource ds) { this.ds = ds; }
  public void setSmsc(Smsc smsc) { this.smsc = smsc; }
  public Smsc getSmsc()  { return smsc; }

  private void bindPeriodPart(PreparedStatement stmt, StatQuery query)
      throws SQLException
  {
    int pos=1;
    if (query.isFromDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
  }
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
  private void bindStatePeriodPart(PreparedStatement stmt, StatQuery query, String id)
      throws SQLException
  {
    int pos=1;
    stmt.setString(pos++, id);
    if (query.isFromDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
  }
  private String prepareStatePeriodPart(StatQuery query, String field) {
    String str = "WHERE "+field+"=? ";
    if (query.isFromDateEnabled() || query.isTillDateEnabled()) str += " AND ";
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
  private String prepareSmeStateQuery(StatQuery query) {
    return SME_STATE_QUERY+prepareStatePeriodPart(query, "systemid")+"GROUP BY errcode";
  }
  private String prepareStateQuery(StatQuery query) {
    return STATE_QUERY+preparePeriodPart(query)+"GROUP BY errcode";
  }
  private String prepareRouteQuery(StatQuery query) {
    return ROUTE_QUERY+preparePeriodPart(query)+"GROUP BY routeid";
  }
  private String prepareRouteStateQuery(StatQuery query) {
    return ROUTE_STATE_QUERY+prepareStatePeriodPart(query, "routeid")+"GROUP BY errcode";
  }

  private void flushStatistics(StatQuery query)
  {
    boolean needFlush = true;
    if (query.isTillDateEnabled()) {
        long till = query.getTillDate().getTime();
        long curr = (new Date()).getTime();
        needFlush = (till >= curr-3600);
    }
    if (needFlush) {
        try {
            smsc.flushStatistics();
        } catch (AdminException e) {
            e.printStackTrace();
        }
    }
  }
  public Statistics getStatistics(StatQuery query)
  {
    flushStatistics(query);

    Connection connection = null;
    stat = new Statistics();
    try
    {
        connection = ds.getConnection();

        processSmsQuery(connection, query);
        processSmeQuery(connection, query);
        processStateQuery(connection, query);
        processRouteQuery(connection, query);
    }
    catch (Exception exc) {
        System.out.println("Operation with DB failed !");
        exc.printStackTrace();
    } finally {
        try { if (connection != null) connection.close(); }
        catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return stat;
  }

  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql)
      throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindPeriodPart(stmt, query);
    return stmt;
  }
  private void processSmsQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    int oldPeriod = 0;
    DateCountersSet dateCounters = null;
    PreparedStatement stmt = getQuery(connection, query, prepareSmsQuery(query));
    ResultSet rs = stmt.executeQuery();

    try
    {
      while (rs.next())
      {
          int newPeriod = rs.getInt(1);
          int hour = calculateHour(newPeriod);
          HourCountersSet hourCounters = new HourCountersSet(
                  rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
                  rs.getInt(6), rs.getInt(7), rs.getInt(8), rs.getInt(9), hour);
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

      if (dateCounters != null) stat.addDateStat(dateCounters);

    } catch (SQLException ex) {
        throw ex;
    } finally {
        if (rs != null) rs.close();
        if (stmt != null) stmt.close();
    }
  }
  private void processSmeQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    PreparedStatement stmt = getQuery(connection, query, prepareSmeQuery(query));
    PreparedStatement errStmt = connection.prepareStatement(prepareSmeStateQuery(query));
    ResultSet rs = stmt.executeQuery();
    ResultSet errRs = null;

    try {
      while (rs.next())
      {
        String smeId = rs.getString(1);
        SmeIdCountersSet set = new SmeIdCountersSet(
            rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
            rs.getInt(6), rs.getInt(7), rs.getInt(8), rs.getInt(9), smeId);

        bindStatePeriodPart(errStmt, query, smeId);
        errRs = errStmt.executeQuery();
        while (errRs.next()) {
          set.addError(new ErrorCounterSet(errRs.getInt(1), errRs.getInt(2)));
        }
        stat.addSmeIdStat(set);
        errRs.close();
      }
    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
      if (errStmt != null) errStmt.close();
    }
  }
  private void processRouteQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    PreparedStatement stmt = getQuery(connection, query, prepareRouteQuery(query));
    PreparedStatement errStmt = connection.prepareStatement(prepareRouteStateQuery(query));
    ResultSet rs = stmt.executeQuery();
    ResultSet errRs = null;

    try
    {
      while (rs.next())
      {
        String routeId = rs.getString(1);
        RouteIdCountersSet set = new RouteIdCountersSet(
            rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
            rs.getInt(6), rs.getInt(7), rs.getInt(8), rs.getInt(9), routeId);

        bindStatePeriodPart(errStmt, query, routeId);
        errRs = errStmt.executeQuery();
        while (errRs.next()) {
          set.addError(new ErrorCounterSet(errRs.getInt(1), errRs.getInt(2)));
        }
        stat.addRouteIdStat(set);
        errRs.close();
      }
    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }
  }
  private void processStateQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    PreparedStatement stmt = getQuery(connection, query, prepareStateQuery(query));
    ResultSet rs = stmt.executeQuery();

    try {
      while (rs.next()) {
        stat.addErrorStat(new ErrorCounterSet(rs.getInt(1), rs.getInt(2)));
      }
    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }
  }

  private boolean needChangeDate(int oldPeriod, int newPeriod)
  {
    String oldStr = Integer.toString(oldPeriod);
    String newStr = Integer.toString(newPeriod);
    oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length()-2));
    newPeriod = Integer.parseInt(newStr.substring(0, newStr.length()-2));
    return (oldPeriod != newPeriod);
  }
  private int calculatePeriod(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    return Integer.parseInt(formatter.format(date));
  }
  private Date calculateDate(int period)
  {
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
  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length()-2));
  }
}
