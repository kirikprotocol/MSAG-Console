package ru.novosoft.smsc.web.controllers.stat;

import mobi.eyeline.util.jsf.components.data_table.model.*;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.stat.*;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatController extends SmscController{

  private static final Logger logger = Logger.getLogger(SmscStatController.class);

  private StatType type = StatType.GENERAL;

  private SmscStatFilter filter = new SmscStatFilter();

  private final SmscStatProvider provider;

  private Statistics statistics;

  private LoadListenerImpl loadListener;

  private boolean loaded;

  private Long providerId;

  private Long categoryId;

  private boolean changed = true;

  public SmscStatController() {
    this.provider = WebContext.getInstance().getSmscStatProvider();
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    setFrom(c.getTime());
  }

  public StatType getType() {
    return type;
  }

  public void setType(StatType type) {
    this.type = type;
  }

  public List<SelectItem> getTypes() {
    StatType[] types = StatType.values();
    List<SelectItem> res = new ArrayList<SelectItem>(types.length);
    for(StatType t : types) {
      res.add(new SelectItem(t, getLocalizedString("smsc.stat.type."+t.toString())));
    }
    return res;
  }


  public String start() {
    if(changed) {
      loaded = false;
      loadListener = null;
      changed = false;
    }
    return null;
  }

  public String clear() {
    loaded = false;
    loadListener = null;
    providerId = null;
    categoryId = null;
    filter.setFrom(null);
    filter.setTill(null);
    return null;
  }

  public boolean isChanged() {
    return changed;
  }

  public void setChanged(boolean changed) {
    this.changed = changed;
  }

  public boolean isShowGeneral(){
    return type == StatType.GENERAL;
  }

  public boolean isShowErrors(){
    return type == StatType.ERRORS;
  }

  public boolean isShowSmes(){
    return type == StatType.SMES;
  }

  public boolean isShowRoutes(){
    return type == StatType.ROUTES;
  }

  public List<SelectItem> getAllProviders() {
    List<SelectItem> l;
    try{
      List<Provider> ps = new ArrayList<Provider>(WebContext.getInstance().getProviderManager().getSettings().getProviders());
      Collections.sort(ps, new Comparator<Provider>() {
        public int compare(Provider o1, Provider o2) {
          return o1.getName().compareTo(o2.getName());
        }
      });
      l = new ArrayList<SelectItem>(ps.size());
      for(Provider p : ps) {
        l.add(new SelectItem(Long.toString(p.getId()), p.getName()));
      }
    }catch (AdminException e){
      addError(e);
      l = Collections.emptyList();
    }
    return l;
  }

  public List<SelectItem> getAllCategories() {
    List<SelectItem> l;
    try{
      List<Category> ps = new ArrayList<Category>(WebContext.getInstance().getCategoryManager().getSettings().getCategories());
      Collections.sort(ps, new Comparator<Category>() {
        public int compare(Category o1, Category o2) {
          return o1.getName().compareTo(o2.getName());
        }
      });
      l = new ArrayList<SelectItem>(ps.size());
      for(Category p : ps) {
        l.add(new SelectItem(Long.toString(p.getId()), p.getName()));
      }
    }catch (AdminException e){
      addError(e);
      l = Collections.emptyList();
    }
    return l;
  }

  private LoadListener load(final Locale locale) {
    LoadListenerImpl listener = null;
    if(!loaded) {
      if(loadListener == null) {
        loadListener = new LoadListenerImpl();
        new Thread() {
          public void run() {
            try{
              statistics = provider.getStatistics(filter, loadListener);
              loaded = true;
            }catch (AdminException e){
              logger.error(e,e);
              loadListener.setLoadError(new ModelException(e.getMessage(locale)));
            }catch (Exception e){
              logger.error(e, e);
              loadListener.setLoadError(new ModelException(e.getLocalizedMessage() != null ? e.getLocalizedMessage() : e.getMessage() != null
                  ? e.getMessage() : "Internal error"));
            }
          }
        }.start();
      }
      listener = loadListener;
    }
    return listener;
  }


  public DataTableModel getGeneral() {

    final Locale locale = getLocale();

    return new PreloadableModel() {
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return load(locale);
      }
      public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        if(statistics == null) {
          return Collections.emptyList();
        }
        List<DateCountersSet> result = new LinkedList<DateCountersSet>();
        List<DateCountersSet> records = statistics.getGeneralStat();
        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          DateCountersSet r = records.get(i);
          result.add(r);
        }
        return result;
      }
      public int getRowsCount() throws ModelException {
        return statistics == null ? 0 : statistics.generalStatSize();
      }
    };
  }

  public CountersSet getTotal() {
    return statistics;
  }

  public DataTableModel getErrors() {

    final Locale locale = getLocale();

    return new PreloadableModel() {
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return load(locale);
      }
      public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        List<ErrorCounterSet> result = new LinkedList<ErrorCounterSet>();
        List<ErrorCounterSet> records = statistics.getErrors();
        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          ErrorCounterSet r = records.get(i);
          result.add(r);
        }
        return result;
      }
      public int getRowsCount() throws ModelException {
        return statistics == null ? 0 : statistics.errorStatSize();
      }
    };
  }

  public DataTableModel getSmeStat() {

    final Locale locale = getLocale();

    return new PreloadableModel() {
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return load(locale);
      }
      public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        List<SmeIdCountersSet> result = new LinkedList<SmeIdCountersSet>();
        List<SmeIdCountersSet> records = statistics.getSmeIdStat();
        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          SmeIdCountersSet r = records.get(i);
          result.add(r);
        }
        return result;
      }
      public int getRowsCount() throws ModelException {
        return statistics == null ? 0 : statistics.smeStatSize();
      }
    };
  }

  private void getFilteredRouteStat(List<RouteIdCountersSet> records) {
    for(RouteIdCountersSet r : statistics.getRouteIdStat()) {
      if((categoryId != null && r.getCategoryId() != categoryId) ||
          (providerId != null && r.getProviderId() != providerId)) {
        continue;
      }
      records.add(r);
    }
  }

  public DataTableModel getRouteStat() {

    final Locale locale = getLocale();

    return new PreloadableModel() {

      final List<RouteIdCountersSet> records = new LinkedList<RouteIdCountersSet>();

      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return load(locale);
      }
      public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        getFilteredRouteStat(records);
        List<RouteIdCountersSet> result = new LinkedList<RouteIdCountersSet>();
        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          RouteIdCountersSet r = records.get(i);
          result.add(r);
        }
        return result;
      }
      public int getRowsCount() throws ModelException {
        return records.size();
      }
    };
  }


  public String getProviderId() {
    return providerId == null ? null : providerId.toString();
  }

  public void setProviderId(String providerId) {
    this.providerId = providerId == null || providerId.length() == 0 ? null : Long.parseLong(providerId);
  }

  public String getCategoryId() {
    return categoryId == null ? null : categoryId.toString();
  }

  public void setCategoryId(String categoryId) {
    this.categoryId = categoryId == null || categoryId.length() == 0 ? null : Long.parseLong(categoryId);
  }



  public Date getFrom() {
    return filter.getFrom();
  }

  public void setFrom(Date from) {
    filter.setFrom(from);
  }

  public Date getTill() {
    return filter.getTill();
  }

  public void setTill(Date till) {
    filter.setTill(till);
  }

  public static enum StatType {
    GENERAL, SMES, ROUTES, ERRORS
  }

  private static class LoadListenerImpl extends LoadListener implements SmscStatLoadListener {
    public void incrementProgress() {
      this.setCurrent(getCurrent() + 1);
    }
  }

}
