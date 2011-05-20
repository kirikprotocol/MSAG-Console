package ru.sibinco.smsx.stats.beans;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.helper.statictable.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.sibinco.smsx.stats.backend.SmsxResults;
import ru.sibinco.smsx.stats.backend.StatRequestManager;
import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;
import ru.sibinco.smsx.stats.backend.datasource.WebDaily;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxDailyTableHelper extends PagedStaticTableHelper{

  private static final Category logger = Category.getInstance(SmsxDailyTableHelper.class);

  private final StatRequestManager requestManager;

  private final SmsxWebFilter filter;

  private Locale locale;


  private final TextColumn dateColumn = new TextColumn("date", "smsx.stat.date", true, 15);
  private final TextColumn regionColumn = new TextColumn("region", "smsx.stat.region", true, 15);
  private final TextColumn trafficTypeColumn = new TextColumn("trafficType", "smsx.stat.trafficType", true, 15);
  private final TextColumn serviceColumn = new TextColumn("serviceName", "smsx.stat.serviceName", false, 15);
  private final TextColumn countColumn = new TextColumn("count", "smsx.stat.outmessages", true,40);

  private ResourceBundle bundle;

  private final int reqId;

  private Set regions;

  public Set getRegions() {
    return regions;
  }

  SmsxDailyTableHelper(String uid, StatRequestManager requestManager, SmsxWebFilter filter, int reqId) {
    super(uid, false);
    setPageSize(20);
    this.reqId = reqId;
    this.requestManager = requestManager;
    this.filter = filter;
    addColumn(dateColumn);
    addColumn(regionColumn);
    addColumn(serviceColumn);
    addColumn(trafficTypeColumn);
    addColumn(countColumn);
  }

  public void setLocale(Locale locale) {
    this.locale = locale;
  }

  private void initResources() {
    if(locale == null) {
      locale = Locale.ENGLISH;
    }
    try{
      bundle = ResourceBundle.getBundle("locales.messages",locale);
    }catch (MissingResourceException e){
      logger.error(e,e);
    }
  }

  private String getLocString(String key) {
    if(bundle == null) {
      return key;
    }
    try{
      return bundle.getString(key);
    }catch (MissingResourceException e){
      logger.error(e,e);
      return key;
    }
  }



  protected void fillTable(int start, int size) throws TableHelperException {
    initResources();
    clear();


    StaticTableHelper.SortOrderElement[] sOs = getSortOrder();
    final StaticTableHelper.SortOrderElement sortOrder = sOs != null && sOs.length>0 ? sOs[0] : null;

    List results = new ArrayList(size);

    try {
      setTotalSize(getSortedRecords(start, size, sortOrder, results, regions = new TreeSet()));
    } catch (StatisticsException e) {
      logger.error(e,e);
      throw new TableHelperException(e);
    }
    Iterator i = results.iterator();
    while(i.hasNext()) {
      WebDaily u = (WebDaily)i.next();
      if(u != null) {
        addRecord(u);
      }
    }

  }

  private void addRecord(WebDaily daily) {
    final Row row = createNewRow();
    row.addCell(dateColumn, new StringCell("", daily.getDate(), false));
    row.addCell(regionColumn, new StringCell("", daily.getRegion(), false));
    row.addCell(serviceColumn, new StringCell("",bundle.getString("smsx.service.SMS_SITE"), false));
    row.addCell(trafficTypeColumn, new StringCell("",bundle.getString(daily.isMsc() ? "smsx.traffic.paid" : "smsx.traffic.free"), false));
    row.addCell(countColumn, new StringCell("",Integer.toString(daily.getCount()), false));
  }

  private static void insert(Object[] infos, Object info, Comparator c) {
    int sRes = Arrays.binarySearch(infos, info, c);
    if (sRes < 0)
      sRes = -sRes - 1;
    if (infos[sRes] != null)
      System.arraycopy(infos, sRes, infos, sRes + 1, infos.length - sRes - 1);
    infos[sRes] = info;
  }


  private int getSortedRecords(final int startPos, final int count, SortOrderElement sortOrder, List rs, final Set regions) throws StatisticsException {
    final Comparator comparator = getComparator(sortOrder);

    final WebDaily infos[] = new WebDaily[startPos + count];
    final int lastIdx = infos.length - 1;

    SmsxResults results = requestManager.getSmsxResult(reqId);
    final int[] total = new int[]{0};
    if(results != null) {
      results.getWebDaily(new Visitor() {
        public boolean visit(Object value) throws StatisticsException {
          WebDaily d = (WebDaily)value;
          regions.add(d.getRegion());
          if(filter != null && !filter.isAllowed(d)) {
            return true;
          }
          ++total[0];
          if (infos[lastIdx] == null || comparator.compare(d, infos[lastIdx]) < 0)
            insert(infos, d, comparator);
          return true;
        }
      });
    }
    rs.addAll(Arrays.asList(infos).subList(startPos, startPos + count));
    return total[0];
  }


  protected int calculateTotalSize() throws TableHelperException {
    return getTotalSize();
  }


  Comparator getComparator(final SortOrderElement sortOrder) {
    return new Comparator() {
      public int compare(Object o1, Object o2) {
        if (o1 == null) return 1;
        if (o2 == null) return -1;
        WebDaily u1 = (WebDaily)o1;
        WebDaily u2 = (WebDaily)o2;
        int mul = (sortOrder == null || sortOrder.getOrderType() == OrderType.ASC) ? 1 : -1;
        if(sortOrder == null || sortOrder.getColumnId().equals("date")){
          return u1.getDate().compareTo(u2.getDate());
        }else if(sortOrder.getColumnId().equals("region")) {
          return u1.getRegion().compareTo(u2.getRegion())*mul;
        }else if(sortOrder.getColumnId().equals("count")) {
          return (u1.getCount() < u2.getCount() ? -1 : u1.getCount() > u2.getCount() ? 1 : 0)*mul;
        }else if(sortOrder.getColumnId().equals("trafficType")) {
          return (!u1.isMsc() && u2.isMsc() ? -1 : u1.isMsc() && !u2.isMsc() ? 1 : 0)*mul;
        }
        return 0;
      }
    };



  }

}
