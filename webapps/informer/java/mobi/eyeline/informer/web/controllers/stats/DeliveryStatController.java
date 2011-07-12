package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.LoadListener;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public abstract class DeliveryStatController extends InformerController{

  private User user;
  private boolean initError = false;
  protected DeliveryStatFilter filter;
  private AggregationType aggregation;

  private boolean init = true;

  private LoadListener loadListener;

  protected final DeliveryStatTotals totals;

  private final Map<Object, AggregatedRecord> recordsMap;

  protected final List<AggregatedRecord> records;


  public DeliveryStatController(DeliveryStatTotals totals) {
    aggregation = AggregationType.DAY;
    filter = new DeliveryStatFilter();
    initUser();
    this.totals = totals;
    records = Collections.synchronizedList(new ArrayList<AggregatedRecord>());
    recordsMap = Collections.synchronizedMap(new TreeMap<Object, AggregatedRecord>());
  }

  protected void initUser() {
    user = getConfig().getUser(getUserName());
    if (user == null) initError = true;
    else if (!user.hasRole(User.INFORMER_ADMIN_ROLE)) {
      filter.setUser(getUserName());
    } else {
      filter.setUser(null);
    }
  }

  public void clearFilter() {
    clearRecords();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskIds(null);
    filter.setRegionId(null);
    setAggregation(AggregationType.DAY);
  }


  public void setUser(User user) {
    this.user = user;
  }

  public User getUser() {
    return user;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void setFilter(DeliveryStatFilter filter) {
    this.filter = filter;
  }

  public AggregationType getAggregation() {
    return aggregation;
  }

  public void setAggregation(AggregationType aggregation) {
    this.aggregation = aggregation;
  }

  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for (AggregationType a : AggregationType.values()) {
      ret.add(new SelectItem(a));
    }
    return ret;
  }

  public boolean isAdmin() {
    return user.hasRole(User.INFORMER_ADMIN_ROLE);
  }

  protected void clearRecords() {
    recordsMap.clear();
    records.clear();
    loaded = false;
    loadListener = null;
    getTotals().reset();
  }

  protected void loadFinished() {
    records.addAll(recordsMap.values());
  }

  protected AggregatedRecord getRecord(Object key) {
    return recordsMap.get(key);
  }

  protected void putRecord(AggregatedRecord newRecord) {
    recordsMap.put(newRecord.getAggregationKey(), newRecord);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem("",""));
    for (User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin(),u.getLogin()));
    }
    Collections.sort(ret, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ret;
  }

  abstract void loadRecords(Configuration config, final Locale locale, LoadListener listener) throws AdminException;

  private boolean loaded = false;

  public boolean isLoaded() {
    return loaded;
  }

  void setLoaded(boolean loaded) {
    this.loaded = loaded;
  }

  public String start() {
    clearRecords();
    init = true;
    return null;
  }

  public boolean isInit() {
    return init;
  }

  protected void load(Configuration config, final Locale locale, LoadListener listener) throws AdminException {
    clearRecords();
    try {
      loadRecords(config, locale, listener);
    }
    catch (AdminException e) {
      clearRecords();
      throw e;
    }
    finally {
      loadFinished();
    }
  }


  public DataTableModel getRecords() {

    final Locale locale = getLocale();

    final Configuration config = getConfig();

    //loadRecords();
    return new PreloadableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, records.get(0).getRecordsComparator(sortOrder));
        }

        List<AggregatedRecord> result = new LinkedList<AggregatedRecord>();
        for (Iterator<AggregatedRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          AggregatedRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            List<AggregatedRecord> innerRecords = r.getInnerRows();
            if (innerRecords != null && !innerRecords.isEmpty() && sortOrder != null) {
              Collections.sort(r.getInnerRows(), innerRecords.get(0).getRecordsComparator(sortOrder));
            }
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return records.size();
      }

      @Override
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder sortOrder) {
        LoadListener listener = null;
        if(!loaded) {
          if(loadListener == null) {
            loadListener = new LoadListener();
            new Thread() {
              public void run() {
                try{
                  DeliveryStatController.this.load(config, locale, loadListener);
                }catch (AdminException e){
                  logger.error(e,e);
                  loadListener.setLoadError(new ModelException(e.getMessage(locale)));
                }catch (Exception e){
                  logger.error(e,e);

                }finally {
                  loaded = true;
                }
              }
            }.start();
          }
          listener = loadListener;
        }
        return listener;
      }
    };
  }

  public boolean isTimeAggregation() {
    return getAggregation() != AggregationType.REGION && getAggregation() != AggregationType.SMSC;
  }

  public boolean isRegionAggregation() {
    return getAggregation() == AggregationType.REGION;
  }

  public boolean isSmscAggregation() {
    return getAggregation() == AggregationType.SMSC;
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    for (int i = 0, recordsSize = records.size(); i < recordsSize; i++) {
      AggregatedRecord r = records.get(i);
      if (i == 0) {
        r.printCSVheader(writer, true);
      }
      r.printWithChildrenToCSV(writer, true);
    }
  }

  public DeliveryStatTotals getTotals() {
    return totals;
  }

  protected Calendar getLastWeekStart() {
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR_OF_DAY,0);
    c.set(Calendar.MINUTE,0);
    c.set(Calendar.SECOND,0);
    c.set(Calendar.MILLISECOND,0);
    c.add(Calendar.DATE,-7);
    return c;
  }
}