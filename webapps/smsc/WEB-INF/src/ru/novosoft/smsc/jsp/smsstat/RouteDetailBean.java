package ru.novosoft.smsc.jsp.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: andrey Date: 07.02.2005 Time: 12:45:18
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.smsstat.RouteIdCountersSet;
import ru.novosoft.smsc.admin.smsstat.StatRouteList;
import ru.novosoft.smsc.admin.smsstat.Statistics;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteQuery;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

public class RouteDetailBean extends IndexBean
{
  // private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private Statistics statistics = null;
  public static final int RESULT_BACK = 4;

  private String mbBack = null;
  private String mbDetail = null;
  private String mbQuickFilter = null;
  private String mbClear = null;
  private String dateFrom = "";
  private String dateTill = "";
  private ArrayList ByRouteId = null;
  private ArrayList RoutesFull = null;
  private boolean filteron = false;
  private Long providerId = null;
  private Long categoryId = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;
  private StatRouteList routes = null;
  protected QueryResultSet routesResult = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;
    providerManager = appContext.getProviderManager();
    categoryManager = appContext.getCategoryManager();
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    RoutesFull = new ArrayList((Collection) session.getAttribute("ByRouteId"));
    if (!filteron) ByRouteId = new ArrayList(RoutesFull);
    dateFrom = (String) session.getAttribute("dateFrom");
    dateTill = (String) session.getAttribute("dateTill");
    if (mbBack != null) {
      try {
        mbBack = null;
        return RESULT_BACK;
      } catch (Exception exc) {
        statistics = null;
        return error(exc.getMessage());
      }
    } else if (mbQuickFilter != null) {
      int dresult = updateFilter();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbClear != null) {
      int dresult = clearFilter();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }

    mbBack = null;
    mbDetail = null;
    return RESULT_OK;
  }

  private int updateFilter()
  {
    filteron = true;
    routes = new StatRouteList();
    mbQuickFilter = null;
    final StatRouteFilter filter = preferences.getStatRouteFilter();

    if (providerId != null)
      filter.setProviderId(providerId);
    else
      filter.setProviderId(null);
    if (categoryId != null)
      filter.setCategoryId(categoryId);
    else
      filter.setCategoryId(null);

    for (Iterator iterator = RoutesFull.iterator(); iterator.hasNext();) {
      RouteIdCountersSet o = (RouteIdCountersSet) iterator.next();
      if (o != null) {
        routes.put(o);
      }
    }
    pageSize = RoutesFull.size();
    routesResult = routes.query(new StatRouteQuery(pageSize, preferences.getStatRouteFilter(), preferences.getStatRoutesSortOrder(), startPosition));
    totalSize = routesResult.getTotalSize();
    ByRouteId.clear();
    for (Iterator iterator = routesResult.iterator(); iterator.hasNext();) {
      StatRouteDataItem o = (StatRouteDataItem) iterator.next();
      RouteIdCountersSet r = new RouteIdCountersSet(o.getAccepted(), o.getRejected(), o.getDelivered(), o.getFailed(), o.getRescheduled(),
              o.getTemporal(), o.getPeak_i(), o.getPeak_o(), o.getRouteID(), o.getProviderId(), o.getCategoryId());
      Collection errors = o.getErrors();
      r.addAllErr(errors);
      ByRouteId.add(r);
    }
    return RESULT_OK;
  }

  private int clearFilter()
  {
    mbClear = null;
    filteron = false;
    final StatRouteFilter filter = preferences.getStatRouteFilter();
    filter.setProviderId(null);
    filter.setCategoryId(null);

    return RESULT_OK;
  }


  public Statistics getStatistics()
  {
    return statistics;
  }

  public String getMbBack()
  {
    return mbBack;
  }

  public void setMbBack(String mbBack)
  {
    this.mbBack = mbBack;
  }

  public String getMbDetail()
  {
    return mbDetail;
  }

  public void setMbDetail(String mbDetail)
  {
    this.mbDetail = mbDetail;
  }

  /* private Date convertStringToDate(String date)
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
   public Collection getByRouteId()
    {
      return ByRouteId;
    }
    */
  public Long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(Long providerId)
  {
    this.providerId = providerId;
  }

  public Long getCategoryId()
  {
    return categoryId;
  }

  public void setCategoryId(Long categoryId)
  {
    this.categoryId = categoryId;
  }

  public void setByRouteId(ArrayList byRouteId)
  {
    this.ByRouteId = byRouteId;
  }

  public ArrayList getByRouteId()
  {
    return ByRouteId;
  }

  public ArrayList getRoutesFull()
  {
    return RoutesFull;
  }

  public void setRoutesFull(ArrayList routesFull)
  {
    RoutesFull = routesFull;
  }

  public String getDateFrom()
  {
    return dateFrom;
  }

  public void setDateFrom(String dateFrom)
  {
    this.dateFrom = dateFrom;
  }

  public String getDateTill()
  {
    return dateTill;
  }

  public void setDateTill(String dateTill)
  {
    this.dateTill = dateTill;
  }

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }

  public String getMbQuickFilter()
  {
    return mbQuickFilter;
  }

  public void setMbQuickFilter(String mbQuickFilter)
  {
    this.mbQuickFilter = mbQuickFilter;
  }

  public Collection getProviders()
  {
    return new SortedList(providerManager.getProviders().values());
  }

  public Collection getCategories()
  {
    return new SortedList(categoryManager.getCategories().values());
  }

  public StatRouteList getRoutes()
  {
    return routes;
  }

  public void setRoutes(StatRouteList routes)
  {
    this.routes = routes;
  }

  public QueryResultSet getRoutesResult()
  {
    return routesResult;
  }

  public void setRoutesResult(QueryResultSet routesResult)
  {
    this.routesResult = routesResult;
  }
  /* -------------------------- StatQuery delegates -------------------------- */
/*  public void setFromDate(String fromDate)
  {
    if (fromDate != null && fromDate.trim().length() > 0) {
      query.setFromDate(convertStringToDate(fromDate));
      query.setFilterFromDate(true);
    }
    else {
      query.setFilterFromDate(false);
    }
  }

  public String getFromDate()
  {
    return (query.isFromDateEnabled()) ?
            convertDateToString(query.getFromDate()) : "";
  }

  public void setTillDate(String tillDate)
  {
    if (tillDate != null && tillDate.trim().length() > 0) {
      query.setTillDate(convertStringToDate(tillDate));
      query.setFilterTillDate(true);
    }
    else {
      query.setFilterTillDate(false);
    }
  }

  public String getTillDate()
  {
    return (query.isTillDateEnabled()) ?
            convertDateToString(query.getTillDate()) : "";
  }
 */
}
