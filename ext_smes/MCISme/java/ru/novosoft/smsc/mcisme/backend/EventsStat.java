package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.AdminException;

import javax.sql.DataSource;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:18:27
 * To change this template use Options | File Templates.
 */
public class EventsStat
{
  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";
  private static final String STAT_QUERY =
    "SELECT period, sum(missed), sum(delivered), sum(failed), sum(notified) FROM MCISME_STAT ";

  private MCISme sme = null;
  private DataSource ds = null;
  private Statistics stat = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }
  public void setMCISme(MCISme sme) {
    this.sme = sme;
  }

  private void bindWherePart(PreparedStatement stmt, StatQuery query)
    throws SQLException
  {
    int pos = 1;
    if (query.isFromDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
  }

  private String prepareWherePart(StatQuery query)
  {
    String str = (query.isFromDateEnabled() || query.isTillDateEnabled()) ? " WHERE " : "";
    if (query.isFromDateEnabled()) {
      str += " period >= ? ";
      if (query.isTillDateEnabled()) str += " AND ";
    }
    if (query.isTillDateEnabled()) str += " period <= ? ";
    return str;
  }

  private String prepareStatQuery(StatQuery query) {
    return STAT_QUERY + prepareWherePart(query) + " GROUP BY period ORDER BY period ASC";
  }

  private void flushStatistics(StatQuery query)
  {
    boolean needFlush = true;
    if (query.isTillDateEnabled()) {
      long till = query.getTillDate().getTime();
      long curr = (new Date()).getTime();
      needFlush = (till >= curr - 3600);
    }
    if (needFlush) {
      try {
        sme.flushStatistics();
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
    try {
      connection = ds.getConnection();

      processStatQuery(connection, query);
    } catch (Exception exc) {
      System.out.println("Operation with DB failed !");
      exc.printStackTrace();
    } finally {
      try {
        if (connection != null) connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
      }
    }
    return stat;
  }

  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql)
    throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindWherePart(stmt, query);
    return stmt;
  }

  private void processStatQuery(Connection connection, StatQuery query)
    throws SQLException
  {
    int oldPeriod = 0;
    DateCountersSet dateCounters = null;
    PreparedStatement stmt = getQuery(connection, query, prepareStatQuery(query));
    ResultSet rs = stmt.executeQuery();

    try {
      while (rs.next()) {
        int newPeriod = rs.getInt(1);
        int hour = calculateHour(newPeriod);
        HourCountersSet hourCounters = new HourCountersSet(
                rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5), hour);
        if (dateCounters == null) { // on first iteration
          Date date = calculateDate(newPeriod);
          dateCounters = new DateCountersSet(date);
        } else if (needChangeDate(oldPeriod, newPeriod)) { // on date changed
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

  private boolean needChangeDate(int oldPeriod, int newPeriod)
  {
    String oldStr = Integer.toString(oldPeriod);
    String newStr = Integer.toString(newPeriod);
    oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length() - 2));
    newPeriod = Integer.parseInt(newStr.substring(0, newStr.length() - 2));
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
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length() - 2));
  }

}
