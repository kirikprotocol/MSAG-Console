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
    " sum(SmsTrOk), sum(SmsTrFailed), sum(SmsTrBilled), sum(UssdTrFromScOk), sum(UssdTrFromScFailed),"+
    " sum(UssdTrFromScBilled), sum(UssdTrFromSmeOk), sum(UssdTrFromSmeFailed), sum(UssdTrFromSmeBilled) ";

  private final static String TOTAL_QUERY =   // group by period
    "SELECT period,"+VALUES_SET+","+TRANS_VALUES_SET+"FROM smppgw_stat_sme ";
  private final static String SME_QUERY =     // group by systemid
    "SELECT systemid,"+VALUES_SET+","+TRANS_VALUES_SET+"FROM smppgw_stat_sme ";
  private static final String ROUTE_QUERY =   // group by routeid
    "SELECT routeid,"+VALUES_SET+"FROM smppgw_stat_route ";

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
  private Statistics getFakeStatistics(StatQuery query) throws Exception
  {
    stat = new Statistics();
    for (int i=0; i<5; i++)
    {
      DateCountersSet dcs = new DateCountersSet(new Date());
      SmeIdCountersSet sics = new SmeIdCountersSet(i,i,i,i,i,i,i,i,i,i,i,i,i,i,"SME"+(5-i));
      RouteIdCountersSet rics = new RouteIdCountersSet(i,i,i,i,i,"Route"+(10-i));
      for (int j=0; j<3; j++)
      {
        HourCountersSet hcs = new HourCountersSet(j,j,j,j,j,j);
        ErrorCounterSet ecs = new ErrorCounterSet(j,j);
        dcs.addHourStat(hcs);
        sics.addError(ecs);
        rics.addError(ecs);
      }
      stat.addDateStat(dcs);
      stat.addSmeIdStat(sics);
      stat.addRouteIdStat(rics);
    }

    return stat;
  }*/

  public Statistics getStatistics(StatQuery query) throws Exception
  {
    if (ds == null) throw new Exception("DataSource is not initialized");
    Connection connection = null;
    stat = new Statistics();
    try
    {
      connection = ds.getConnection();
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
    //return getFakeStatistics(query);
  }

  private void bindWherePart(PreparedStatement stmt, StatQuery query, boolean provider)
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
  private String prepareWherePart(StatQuery query, boolean provider)
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

  private String prepareTotalQuery(StatQuery query) {
    return TOTAL_QUERY+prepareWherePart(query, true)+"GROUP BY period ORDER BY period ASC";
  }
  private String prepareSmeQuery(StatQuery query) {
    return SME_QUERY+prepareWherePart(query, true)+"GROUP BY systemid";
  }
  private String prepareRouteQuery(StatQuery query) {
    return ROUTE_QUERY+prepareWherePart(query, true)+"GROUP BY routeid";
  }
  private String prepareSmeStateQuery(StatQuery query) {
    return SME_STATE_QUERY+prepareWherePart(query, false)+"GROUP BY systemid, errcode";
  }
  private String prepareRouteStateQuery(StatQuery query) {
    return ROUTE_STATE_QUERY+prepareWherePart(query, false)+"GROUP BY routeid, errcode";
  }

  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql, boolean provider)
      throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindWherePart(stmt, query, provider);
    return stmt;
  }

  private void processSmeQuery(Connection connection, StatQuery query)
      throws SQLException, Exception
  {
    HashMap countersForSme = new HashMap();
    PreparedStatement stmt = getQuery(connection, query, prepareTotalQuery(query), true);
    ResultSet rs = stmt.executeQuery();

    try
    {
      DateCountersSet dateCounters = null;
      int oldPeriod = 0;

      while (rs.next())
      {
          int newPeriod = rs.getInt(1);
          int hour = calculateHour(newPeriod);
          HourCountersSet hourCounters = new HourCountersSet(
                  rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                  rs.getLong(7), rs.getLong(8), rs.getLong(9), rs.getLong(10), rs.getLong(11),
                  rs.getLong(12), rs.getLong(13), rs.getLong(14), rs.getLong(15), hour);
          if (dateCounters == null) { // on first iteration
              Date date = calculateDate(newPeriod);
              dateCounters = new DateCountersSet(date);
          }
          else if (needChangeDate(oldPeriod, newPeriod)) { // date is unchanged
              stat.addDateStat(dateCounters);
              Date date = calculateDate(newPeriod);
              dateCounters = new DateCountersSet(date);
          }
          dateCounters.addHourStat(hourCounters);
          oldPeriod = newPeriod;
      }
      if (dateCounters != null) stat.addDateStat(dateCounters);
      rs.close(); stmt.close();

      stmt = getQuery(connection, query, prepareSmeQuery(query), true);
      rs = stmt.executeQuery();
      while (rs.next())
      {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        countersForSme.put(smeId,
            new SmeIdCountersSet(rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                                 rs.getLong(7), rs.getLong(8), rs.getLong(9), rs.getLong(10), rs.getLong(11),
                                 rs.getLong(12), rs.getLong(13), rs.getLong(14), rs.getLong(15), smeId));
      }
      rs.close(); stmt.close();

      stmt = getQuery(connection, query, prepareSmeStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next())
      {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        Object obj = countersForSme.get(smeId);
        if (obj != null && obj instanceof SmeIdCountersSet)
          ((SmeIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getLong(3)));
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
            new RouteIdCountersSet(rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                                   routeId));
      }
      rs.close(); stmt.close();

      stmt = getQuery(connection, query, prepareRouteStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        Object obj = countersForRoute.get(routeId);
        if (obj != null && obj instanceof RouteIdCountersSet)
          ((RouteIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getLong(3)));
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
