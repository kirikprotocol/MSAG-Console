package ru.novosoft.smsc.jsp.smsc.routes;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class Index extends IndexBean
{
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  protected QueryResultSet routes = null;

  protected String editRouteId = null;

  protected String[] checkedRouteIds = new String[0];
  protected Set checkedRouteIdsSet = new HashSet();

  protected String filterSelect = null;
  protected String queryName = null;
  protected String querySubj = null;
  protected String queryMask = null;
  protected String querySMEs = null;
  protected String queryProvider = null;
  protected String queryCategory = null;
  protected String queryProviderId = null;
  protected String queryCategoryId = null;
  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbEdit = null;
  protected String mbSave = null;
  protected String mbRestore = null;
  protected String mbLoad = null;
  private String mbQuickFilter = null;
  protected String mbClear = null;

  protected boolean initialized = false;

  protected String[] srcChks = null;
  protected String[] srcMasks = null;
  protected String[] dstChks = null;
  protected String[] dstMasks = null;
  protected String[] smeChks = null;
  protected String[] names = null;
  protected String[] providerNames = null;
  protected String[] categoryNames = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    providerManager = appContext.getProviderManager();
    categoryManager = appContext.getCategoryManager();
    // providers=providerManager.getProviders();
    //providers=providerManager.getProviders();
    pageSize = preferences.getRoutesPageSize();
    if (sort != null)
      preferences.getRoutesSortOrder().set(0, sort);
    else
      sort = (String) preferences.getRoutesSortOrder().get(0);

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    routes = new EmptyResultSet();
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      final RouteFilter routesFilter = preferences.getRoutesFilter(appContext);
      srcChks = routesFilter.getSourceSubjectNames();
      srcMasks = routesFilter.getSourceMaskStrings();
      dstChks = routesFilter.getDestinationSubjectNames();
      dstMasks = routesFilter.getDestinationMaskStrings();
      smeChks = routesFilter.getSmeIds();
      names = routesFilter.getNames();
      providerNames = routesFilter.getProviders();
      categoryNames = routesFilter.getCategories();
      try {
        queryName = names[0];
      } catch (Exception e) {
        queryName = "";
      }
      try {
        querySubj = srcChks[0];
      } catch (Exception e) {
        querySubj = "";
      }
      try {
        queryMask = srcMasks[0];
      } catch (Exception e) {
        queryMask = "";
      }
      try {
        querySMEs = smeChks[0];
      } catch (Exception e) {
        querySMEs = "";
      }
      try {
        queryProvider = providerNames[0];
      } catch (Exception e) {
        queryProvider = "";
      }
      try {
        queryCategory = categoryNames[0];
      } catch (Exception e) {
        queryCategory = "";
      }
    }


    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null) {
      int dresult = deleteRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbSave != null) {
      int dresult = saveRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbLoad != null) {
      int dresult = loadRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbRestore != null) {
      int dresult = restoreRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbQuickFilter != null) {
      int dresult = updateFilter();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbClear != null) {
      int dresult = clearFilter();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }

    logger.debug("Routes.Index - process with sorting [" + (String) preferences.getRoutesSortOrder().get(0) + "]");
    routes = routeSubjectManager.getRoutes().query(new RouteQuery(pageSize, preferences.getRoutesFilter(appContext), preferences.getRoutesSortOrder(), startPosition));
    totalSize = routes.getTotalSize();

    checkedRouteIdsSet.addAll(Arrays.asList(checkedRouteIds));

    return RESULT_OK;
  }

  private int updateFilter()
  {
    try {
      final RouteFilter routesFilter = preferences.getRoutesFilter(appContext);
      routesFilter.setIntersection(1);
      if (queryName != null)
        routesFilter.setNames(new String[]{queryName.toLowerCase()});
      else
        routesFilter.setNames(new String[0]);
      if (querySubj != null) {
        routesFilter.setSourceSubjectNames(new String[]{querySubj.toLowerCase()});
        routesFilter.setDestinationSubjectNames(new String[]{querySubj.toLowerCase()});
      }
      else {
        routesFilter.setSourceSubjectNames(new String[0]);
        routesFilter.setDestinationSubjectNames(new String[0]);
      }
      if (queryMask != null) {
        routesFilter.setSourceMaskStrings(new String[]{queryMask});
        routesFilter.setDestinationMaskStrings(new String[]{queryMask});
      }
      else {
        routesFilter.setSourceMaskStrings(new String[0]);
        routesFilter.setDestinationMaskStrings(new String[0]);
      }
      if (querySMEs != null)
        routesFilter.setSmeIds(new String[]{querySMEs.toLowerCase()});
      else
        routesFilter.setSmeIds(new String[0]);
      if (queryProvider != null)
        routesFilter.setProviders(new String[]{queryProvider.toLowerCase()});
      else
        routesFilter.setProviders(new String[0]);
      if (queryCategory != null)
        routesFilter.setCategories(new String[]{queryCategory.toLowerCase()});
      else
        routesFilter.setCategories(new String[0]);

      /*    if ("5".equals(filterSelect))
                  routesFilter.setSmeIds(new String[]{queryName});
    */
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.CantUpdateFilter, e);
    }
    return RESULT_OK;
  }

  private int clearFilter()
  {
    try {
      final RouteFilter routesFilter = preferences.getRoutesFilter(appContext);
      // routesFilter.setIntersection(false);
      routesFilter.setNames(new String[0]);
      routesFilter.setProviders(new String[0]);
      routesFilter.setCategories(new String[0]);
      routesFilter.setSourceSubjectNames(new String[0]);
      routesFilter.setDestinationSubjectNames(new String[0]);
      routesFilter.setSourceMaskStrings(new String[0]);
      routesFilter.setDestinationMaskStrings(new String[0]);
      routesFilter.setSmeIds(new String[0]);
    } catch (AdminException e) {
      return error(SMSCErrors.error.routes.CantUpdateFilter, e);
    }
    return RESULT_OK;
  }


  protected int deleteRoutes()
  {
    for (int i = 0; i < checkedRouteIds.length; i++) {
      String routeId = checkedRouteIds[i];
      routeSubjectManager.getRoutes().remove(routeId);
      journalAppend(SubjectTypes.TYPE_route, routeId, Actions.ACTION_DEL);
      appContext.getStatuses().setRoutesChanged(true);
    }
    checkedRouteIds = new String[0];
    checkedRouteIdsSet.clear();
    return RESULT_OK;
  }

  protected int saveRoutes() // saves temporal configuration
  {
    try {
      routeSubjectManager.save();
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantSave, exc.getMessage());
    }
    appContext.getStatuses().setRoutesSaved(true);
    appContext.getStatuses().setRoutesRestored(true);
    return RESULT_OK;
  }

  public Date getRestoreFileDate() // get modified date of temporal config
  {
    try {
      return routeSubjectManager.getRestoreFileDate();
    } catch (AdminException e) {
      logger.debug("Could not get date of saved temporal config file", e);
      return null;
    }
  }

  protected int restoreRoutes() // loads saved configuration
  {
    try {
      routeSubjectManager.restore();
      journalAppend(SubjectTypes.TYPE_route, null, Actions.ACTION_RESTORE);
      journalAppend(SubjectTypes.TYPE_subject, null, Actions.ACTION_RESTORE);
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantRestore, exc.getMessage());
    }
    appContext.getStatuses().setSubjectsChanged(true);
    appContext.getStatuses().setRoutesChanged(true);
    appContext.getStatuses().setRoutesSaved(true);
    appContext.getStatuses().setRoutesRestored(true);
    appContext.getStatuses().setRoutesLoaded(false);
    return RESULT_OK;
  }

  protected int loadRoutes() // loads applied configuration
  {
    try {
      routeSubjectManager.load();
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantLoad, exc.getMessage());
    }
    appContext.getJournal().clear(SubjectTypes.TYPE_route);
    appContext.getJournal().clear(SubjectTypes.TYPE_subject);
    appContext.getStatuses().setSubjectsChanged(false);
    appContext.getStatuses().setRoutesChanged(false);
    appContext.getStatuses().setRoutesRestored(false);
    appContext.getStatuses().setRoutesLoaded(false);
    return RESULT_OK;
  }

  public Date getLoadFileDate() // get modified date of primary config
  {
    try {
      return routeSubjectManager.getLoadFileDate();
    } catch (AdminException e) {
      logger.debug("Could not get date of saved config file", e);
      return null;
    }
  }

  public boolean isRouteChecked(String alias)
  {
    return checkedRouteIdsSet.contains(alias);
  }

  public QueryResultSet getRoutes()
  {
    return routes;
  }

  /**
   * ***************** properties ************************
   */

  public String getEditRouteId()
  {
    return editRouteId;
  }

  public void setEditRouteId(String editRouteId)
  {
    this.editRouteId = editRouteId;
  }

  public String[] getCheckedRouteIds()
  {
    return checkedRouteIds;
  }

  public void setCheckedRouteIds(String[] checkedRouteIds)
  {
    this.checkedRouteIds = checkedRouteIds;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbRestore()
  {
    return mbRestore;
  }

  public void setMbRestore(String mbRestore)
  {
    this.mbRestore = mbRestore;
  }

  public String getMbLoad()
  {
    return mbLoad;
  }

  public void setMbLoad(String mbLoad)
  {
    this.mbLoad = mbLoad;
  }

  public String getQueryName()
  {
    return queryName;
  }

  public void setQueryName(String queryName)
  {
    this.queryName = queryName;
  }

  public String getQuerySubj()
  {
    return querySubj;
  }

  public void setQuerySubj(String querySubj)
  {
    this.querySubj = querySubj;
  }

  public String getQueryMask()
  {
    return queryMask;
  }

  public void setQueryMask(String queryMask)
  {
    this.queryMask = queryMask;
  }

  public String getQuerySMEs()
  {
    return querySMEs;
  }

  public void setQuerySMEs(String querySMEs)
  {
    this.querySMEs = querySMEs;
  }

  public String getQueryCategory()
  {
    return queryCategory;
  }

  public void setQueryCategory(String queryCategory)
  {
    this.queryCategory = queryCategory;
  }

  public String getQueryProvider()
  {
    return queryProvider;
  }

  public void setQueryProvider(String queryProvider)
  {
    this.queryProvider = queryProvider;
  }

  public String getFilterSelect()
  {
    return filterSelect;
  }

  public void setFilterSelect(String filterSelect)
  {
    this.filterSelect = filterSelect;
  }

  public String getMbQuickFilter()
  {
    return mbQuickFilter;
  }

  public void setMbQuickFilter(String mbQuickFilter)
  {
    this.mbQuickFilter = mbQuickFilter;
  }

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }


}
