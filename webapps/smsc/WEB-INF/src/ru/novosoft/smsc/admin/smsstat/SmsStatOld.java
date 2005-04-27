package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:41:41 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;

public class SmsStatOld
{
  private final static String ERRORS_SET =
          " errcode, sum(counter) ";
  private final static String VALUES_SET =
          " sum(accepted), sum(rejected), sum(delivered), sum(failed)," +
          " sum(rescheduled), sum(temporal), max(peak_i), max(peak_o) ";

  private final static String SMS_QUERY = // group by period
          "SELECT period," + VALUES_SET + "FROM sms_stat_sms ";
  private final static String SME_QUERY = // group by systemid
          "SELECT systemid," + VALUES_SET + "FROM sms_stat_sme ";
  private static final String ROUTE_QUERY = // group by routeid
          "SELECT routeid," + VALUES_SET + "FROM sms_stat_route ";

  private static final String STATE_QUERY = // group by errcode
          "SELECT" + ERRORS_SET + "FROM sms_stat_state ";
  private static final String SME_STATE_QUERY = // group by systemid, errcode
          "SELECT systemid," + ERRORS_SET + "FROM sms_stat_sme_state ";
  private static final String ROUTE_STATE_QUERY = // group by routeid, errcode
          "SELECT routeid," + ERRORS_SET + "FROM sms_stat_route_state ";

  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";

  private DataSource ds = null;
  private Smsc smsc = null;
  protected RouteSubjectManager routeSubjectManager = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;

  private Statistics stat = null;

  public void setDataSource(DataSource ds)
  {
    this.ds = ds;
  }

  public void setSmsc(Smsc smsc)
  {
    this.smsc = smsc;
  }

  public Smsc getSmsc()
  {
    return smsc;
  }

  public RouteSubjectManager getRouteSubjectManager()
  {
    return routeSubjectManager;
  }

  public void setRouteSubjectManager(RouteSubjectManager routeSubjectManager)
  {
    this.routeSubjectManager = routeSubjectManager;
  }

  public ProviderManager getProviderManager()
  {
    return providerManager;
  }

  public void setProviderManager(ProviderManager providerManager)
  {
    this.providerManager = providerManager;
  }

  public CategoryManager getCategoryManager()
  {
    return categoryManager;
  }

  public void setCategoryManager(CategoryManager categoryManager)
  {
    this.categoryManager = categoryManager;
  }

  private void bindPeriodPart(PreparedStatement stmt, StatQuery query)
          throws SQLException
  {
    int pos = 1;
    if (query.isFromDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
  }

  private String preparePeriodPart(StatQuery query)
  {
    String str = (query.isFromDateEnabled() ||
            query.isTillDateEnabled()) ? " WHERE " : "";
    if (query.isFromDateEnabled()) {
      str += " period >= ? ";
      if (query.isTillDateEnabled()) str += " AND ";
    }
    if (query.isTillDateEnabled()) str += " period <= ? ";
    return str;
  }

  private String prepareSmsQuery(StatQuery query)
  {
    return SMS_QUERY + preparePeriodPart(query) + "GROUP BY period ORDER BY period ASC";
  }

  private String prepareSmeQuery(StatQuery query)
  {
    return SME_QUERY + preparePeriodPart(query) + "GROUP BY systemid";
  }

  private String prepareSmeStateQuery(StatQuery query)
  {
    return SME_STATE_QUERY + preparePeriodPart(query) + "GROUP BY systemid, errcode";
  }

  private String prepareStateQuery(StatQuery query)
  {
    return STATE_QUERY + preparePeriodPart(query) + "GROUP BY errcode";
  }

  private String prepareRouteQuery(StatQuery query)
  {
    return ROUTE_QUERY + preparePeriodPart(query) + "GROUP BY routeid";
  }

  private String prepareRouteStateQuery(StatQuery query)
  {
    return ROUTE_STATE_QUERY + preparePeriodPart(query) + "GROUP BY routeid, errcode";
  }

  private void flushStatistics(StatQuery query) throws AdminException
  {
    if (smsc == null || smsc.getInfo().getStatus() != ServiceInfo.STATUS_RUNNING) return;

    boolean needFlush = true;
    if (query.isTillDateEnabled()) {
      long till = query.getTillDate().getTime();
      long curr = (new Date()).getTime();
      needFlush = (till >= curr - 3600);
    }
    if (needFlush) smsc.flushStatistics();
  }

  public Statistics getStatistics(StatQuery query) throws AdminException
  {
    flushStatistics(query);

    Connection connection = null;
    stat = new Statistics();
    try {
      connection = ds.getConnection();

      processSmsQuery(connection, query);
      processSmeQuery(connection, query);
      processStateQuery(connection, query);
      processRouteQuery(connection, query);
    } catch (SQLException exc) {
      final String errorMessage = "Operation with DB failed !";
      System.out.println(errorMessage);
      exc.printStackTrace();
      throw new AdminException(errorMessage + " Details: " + exc.getMessage());
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
    bindPeriodPart(stmt, query);
    return stmt;
  }

  private void processSmsQuery(Connection connection, StatQuery query)
          throws SQLException, AdminException
  {
    int oldPeriod = 0;
    DateCountersSet dateCounters = null;
    PreparedStatement stmt = getQuery(connection, query, prepareSmsQuery(query));
    ResultSet rs = stmt.executeQuery();

    try {
      while (rs.next()) {
        int newPeriod = rs.getInt(1);
        int hour = calculateHour(newPeriod);
        HourCountersSet hourCounters = new HourCountersSet(rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
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
    HashMap countersForSme = new HashMap();
    PreparedStatement stmt = null;
    ResultSet rs = null;

    try {
      stmt = getQuery(connection, query, prepareSmeQuery(query));
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        countersForSme.put(smeId,
                new SmeIdCountersSet(rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
                        rs.getInt(6), rs.getInt(7), rs.getInt(8), rs.getInt(9), smeId));
      }
      rs.close();
      stmt.close();

      stmt = getQuery(connection, query, prepareSmeStateQuery(query));
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        Object obj = countersForSme.get(smeId);
        if (obj != null && obj instanceof SmeIdCountersSet)
          ((SmeIdCountersSet) obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getInt(3)));
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

    try {
      stmt = getQuery(connection, query, prepareRouteQuery(query));
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        Provider provider = null;
        Category category = null;
        final Route r = routeSubjectManager.getRoutes().get(routeId);
        if (null != r) {
          provider = providerManager.getProvider(new Long(r.getProviderId()));
          category = categoryManager.getCategory(new Long(r.getCategoryId()));
        }
        if (provider == null) provider = new Provider(-1, "");
        if (category == null) category = new Category(-1, "");

        countersForRoute.put(routeId,
                new RouteIdCountersSet(rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5),
                        rs.getInt(6), rs.getInt(7), rs.getInt(8), rs.getInt(9), routeId, provider, category));
      }
      rs.close();
      stmt.close();

      stmt = getQuery(connection, query, prepareRouteStateQuery(query));
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        Object obj = countersForRoute.get(routeId);
        if (obj != null && obj instanceof RouteIdCountersSet)
          ((RouteIdCountersSet) obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getInt(3)));
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
    oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length() - 2));
    newPeriod = Integer.parseInt(newStr.substring(0, newStr.length() - 2));
    return (oldPeriod != newPeriod);
  }

  private int calculatePeriod(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    return Integer.parseInt(formatter.format(date));
  }

  private Date calculateDate(int period) throws AdminException
  {
    Date converted = new Date();
    String str = Integer.toString(period);
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
      converted = formatter.parse(str);
    } catch (ParseException e) {
      e.printStackTrace();
      throw new AdminException("Failed to convert period to date. " + e.getMessage());
    }
    return converted;
  }

  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length() - 2));
  }
}
