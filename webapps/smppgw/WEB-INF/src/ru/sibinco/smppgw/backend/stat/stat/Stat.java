package ru.sibinco.smppgw.backend.stat.stat;

import javax.sql.DataSource;
import java.util.Date;
import java.util.HashMap;
import java.util.Collection;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.08.2004
 * Time: 16:00:56
 * To change this template use File | Settings | File Templates.
 */
public class Stat
{
  private final static String ERRORS_SET =
    " errcode, sum(counter) ";
  private final static String VALUES_SET =
    " sum(accepted), sum(rejected), sum(delivered), sum(temperror), sum(permerror) ";
  private final static String TRANS_VALUES_SET =
    " sum(SmsTrOk), sum(SmsTrFailed), sum(UssdTrFromScOk), sum(UssdTrFromScFailed),"+
    " sum(UssdTrFromScBilled), sum(UssdTrFromSmeOk), sum(UssdTrFromSmeFailed), sum(UssdTrFromSmeBilled) ";

  private final static String SMS_QUERY =     // group by period
    "SELECT period,"+VALUES_SET+"FROM smppgw_stat ";
  private final static String SME_QUERY =     // group by systemid
    "SELECT systemid,"+VALUES_SET+","+TRANS_VALUES_SET+"FROM smppgw_stat_sme ";
  private static final String ROUTE_QUERY =   // group by routeid
    "SELECT routeid,"+VALUES_SET+"FROM smppgw_stat_route ";

  private static final String STATE_QUERY =       // group by errcode
    "SELECT"+ERRORS_SET+"FROM smppgw_stat_errors ";
  private static final String SME_STATE_QUERY =   // group by systemid, errcode
    "SELECT systemid,"+ERRORS_SET+"FROM smppgw_stat_sme_errors ";
  private static final String ROUTE_STATE_QUERY = // group by routeid, errcode
    "SELECT routeid,"+ERRORS_SET+"FROM smppgw_stat_route_errors ";

  private DataSource ds = null;
  private Statistics stat = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  /*
  private Statistics getFakeStatistics() throws Exception
  {
    stat = new Statistics();
    for (int i=0; i<5; i++)
    {
      DateCountersSet dcs = new DateCountersSet(new Date());
      SmeIdCountersSet sics = new SmeIdCountersSet(i,i,i,i,i,i,i,i,i,i,i,i,i,"SME"+(5-i));
      RouteIdCountersSet rics = new RouteIdCountersSet(i,i,i,i,i,"Route"+(10-i));
      for (int j=0; j<3; j++)
      {
        HourCountersSet hcs = new HourCountersSet(j,j,j,j,j,j);
        ErrorCounterSet ecs = new ErrorCounterSet(j,j);
        dcs.addHourStat(hcs);
        sics.addError(ecs);
        rics.addError(ecs);
        stat.addErrorStat(ecs);
      }
      stat.addDateStat(dcs);
      stat.addSmeIdStat(sics);
      stat.addRouteIdStat(rics);
    }
    return stat;
  }
  */

  public Statistics getStatistics(StatQuery query) throws Exception
  {
    //return getFakeStatistics();
    if (ds == null) throw new Exception("DataSource is not initialized");
    Connection connection = null;
    stat = new Statistics(); stat.setFull(false);
    try
    {
      connection = ds.getConnection();

      if (query.getProviderId() == StatQuery.ALL_PROVIDERS) {
        processSmsQuery(connection, query);
        processStateQuery(connection, query);
        stat.setFull(true);
      }
      processSmeQuery(connection, query);
      processRouteQuery(connection, query);
    }
    catch (SQLException exc) {
      final String errorMessage = "Operation with DB failed !";
      System.out.println(errorMessage); exc.printStackTrace();
      throw new Exception(errorMessage+" Details: "+exc.getMessage());
    } finally {
      try { if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return stat;
  }

  private void bindPeriodPart(PreparedStatement stmt, StatQuery query, boolean provider)
      throws SQLException
  {
    provider = (provider && (query.getProviderId() != StatQuery.ALL_PROVIDERS));
    int pos=1;
    if (query.isFromDateEnabled())
      stmt.setInt (pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt (pos++, calculatePeriod(query.getTillDate()));
    if (provider)
      stmt.setLong(pos++, query.getProviderId());
  }
  private String preparePeriodPart(StatQuery query, boolean provider)
  {
    provider = (provider && (query.getProviderId() != StatQuery.ALL_PROVIDERS));
    String str = (query.isFromDateEnabled() ||
                  query.isTillDateEnabled() || provider) ? " WHERE ":"";
    if (query.isFromDateEnabled()) {
      str += " period >= ? ";
      if (query.isTillDateEnabled() || provider) str += " AND ";
    }
    if (query.isTillDateEnabled()) {
      str += " period <= ? ";
      if (provider) str += " AND ";
    }
    if (provider) {
      str += " providerid=? ";
    }
    return str;
  }

  private String prepareSmsQuery(StatQuery query) {
    return SMS_QUERY+preparePeriodPart(query, false)+"GROUP BY period ORDER BY period ASC";
  }
  private String prepareSmeQuery(StatQuery query) {
    return SME_QUERY+preparePeriodPart(query, true)+"GROUP BY systemid";
  }
  private String prepareRouteQuery(StatQuery query) {
    return ROUTE_QUERY+preparePeriodPart(query, true)+"GROUP BY routeid";
  }
  private String prepareStateQuery(StatQuery query) {
    return STATE_QUERY+preparePeriodPart(query, false)+"GROUP BY errcode";
  }
  private String prepareSmeStateQuery(StatQuery query) {
    return SME_STATE_QUERY+preparePeriodPart(query, false)+"GROUP BY systemid, errcode";
  }
  private String prepareRouteStateQuery(StatQuery query) {
    return ROUTE_STATE_QUERY+preparePeriodPart(query, false)+"GROUP BY routeid, errcode";
  }

  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql, boolean provider)
      throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindPeriodPart(stmt, query, provider);
    return stmt;
  }

  private void processSmsQuery(Connection connection, StatQuery query)
      throws SQLException, Exception
  {
    int oldPeriod = 0;
    DateCountersSet dateCounters = null;
    PreparedStatement stmt = getQuery(connection, query, prepareSmsQuery(query), false);
    ResultSet rs = stmt.executeQuery();

    try
    {
      while (rs.next())
      {
          int newPeriod = rs.getInt(1);
          int hour = calculateHour(newPeriod);
          HourCountersSet hourCounters = new HourCountersSet(
                  rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5), rs.getInt(6), hour);
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
    HashMap countersForSme = new HashMap();
    PreparedStatement stmt = null;
    ResultSet rs = null;

    try
    {
      stmt = getQuery(connection, query, prepareSmeQuery(query), true);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        countersForSme.put(smeId,
            new SmeIdCountersSet(rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5), rs.getInt(6),
                                 rs.getInt(7), rs.getInt(8), rs.getInt(9), rs.getInt(10), rs.getInt(11),
                                 rs.getInt(12), rs.getInt(13), rs.getInt(14), smeId));
      }
      rs.close(); stmt.close();

      stmt = getQuery(connection, query, prepareSmeStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        Object obj = countersForSme.get(smeId);
        if (obj != null && obj instanceof SmeIdCountersSet)
          ((SmeIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getInt(3)));
      }

    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }

    Collection counters = countersForSme.values();
    if (counters != null) stat.addSmeIdCollection(counters);
  }

  private void processRouteQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    HashMap countersForRoute = new HashMap();
    PreparedStatement stmt = null;
    ResultSet rs = null;

    try
    {
      stmt = getQuery(connection, query, prepareRouteQuery(query), true);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        countersForRoute.put(routeId,
            new RouteIdCountersSet(rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5), rs.getInt(6), routeId));
      }
      rs.close(); stmt.close();

      stmt = getQuery(connection, query, prepareRouteStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        Object obj = countersForRoute.get(routeId);
        if (obj != null && obj instanceof RouteIdCountersSet)
          ((RouteIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getInt(3)));
      }

    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }

    Collection counters = countersForRoute.values();
    if (counters != null) stat.addRouteIdCollection(counters);
  }

  private void processStateQuery(Connection connection, StatQuery query)
      throws SQLException
  {
    PreparedStatement stmt = getQuery(connection, query, prepareStateQuery(query), false);
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

  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";

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
  private Date calculateDate(int period) throws Exception
  {
    Date converted = new Date();
    String str = Integer.toString(period);
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
      converted = formatter.parse(str);
    } catch (ParseException e) {
      e.printStackTrace();
      throw new Exception("Failed to convert period to date. "+e.getMessage());
    }
    return converted;
  }
  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length()-2));
  }

}
