package ru.sibinco.sponsored.stats.beans;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.helper.statictable.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.sibinco.sponsored.stats.backend.SponsoredRecord;
import ru.sibinco.sponsored.stats.backend.StatRequestManager;
import ru.sibinco.sponsored.stats.backend.StatisticsException;
import ru.sibinco.sponsored.stats.backend.Visitor;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SponsorResultsTableHelper extends PagedStaticTableHelper{

  private static final Category logger = Category.getInstance(SponsorResultsTableHelper.class);

  private StatRequestManager requestManager;

  private SponsoredResultFilter filter;

  private Locale locale;

  private final TextColumn addressColumn = new TextColumn("address", "sponsored.address", true, 15);
  private final TextColumn bonusColumn = new TextColumn("bonus", "smsx.stat.bonus", true, 85);

  private int reqId;


  SponsorResultsTableHelper(String uid, StatRequestManager requestManager, SponsoredResultFilter filter, int reqId) {
    super(uid, false);
    setPageSize(20);
    this.reqId = reqId;
    this.requestManager = requestManager;
    this.filter = filter;
    addColumn(addressColumn);
    addColumn(bonusColumn);
  }

  public void setLocale(Locale locale) {
    this.locale = locale;
  }

  private ResourceBundle bundle;


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
      setTotalSize(getSortedRecords(start, size-1, sortOrder, results));
    } catch (StatisticsException e) {
      logger.error(e,e);
      throw new TableHelperException(e);
    }
    Iterator i = results.iterator();
    float total = 0;
    while(i.hasNext()) {
      SponsoredRecord u = (SponsoredRecord)i.next();
      if(u != null) {
        addRecord(u);
        total+=u.getBonus();
      }
    }
    final Row row = createNewRow();
    row.addCell(addressColumn, new StringCell("", getLocString("sponsored.bonus.total"), false));
    row.addCell(bonusColumn, new StringCell("", Float.toString(total), false));
  }

  private void addRecord(SponsoredRecord record) {
    final Row row = createNewRow();
    row.addCell(addressColumn, new StringCell("", record.getAddress(), false));
    row.addCell(bonusColumn, new StringCell("", Float.toString(record.getBonus()), false));
  }

  private static void insert(Object[] infos, Object info, Comparator c) {
    int sRes = Arrays.binarySearch(infos, info, c);

    if (sRes < 0)
      sRes = -sRes - 1;

    if (infos[sRes] != null)
      System.arraycopy(infos, sRes, infos, sRes + 1, infos.length - sRes - 1);

    infos[sRes] = info;
  }


  private int getSortedRecords(final int startPos, final int count, SortOrderElement sortOrder, List rs) throws StatisticsException {
    final Comparator comparator = getComparator(sortOrder);

    final SponsoredRecord infos[] = new SponsoredRecord[startPos + count];
    final int lastIdx = infos.length - 1;

    final int[] total = new int[]{0};

    requestManager.getSponsoredResult(reqId, new Visitor() {
      public boolean visit(Object value) throws StatisticsException {
        if(filter != null && !filter.isAllowed((SponsoredRecord)value)) {
          return true;
        }
        ++total[0];
        if (infos[lastIdx] == null || comparator.compare(value, infos[lastIdx]) < 0)
          insert(infos, value, comparator);
        return true;
      }
    });

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
        SponsoredRecord u1 = (SponsoredRecord)o1;
        SponsoredRecord u2 = (SponsoredRecord)o2;
        int mul = (sortOrder == null || sortOrder.getOrderType() == OrderType.ASC) ? 1 : -1;
        if(sortOrder == null || sortOrder.getColumnId().equals("address")) {
          return u1.getAddress().compareTo(u2.getAddress())*mul;
        }else if(sortOrder.getColumnId().equals("bonus")) {
          return (u1.getBonus() < u2.getBonus() ? -1 : u1.getBonus() > u2.getBonus() ? 1 : 0)*mul;
        }
        return 0;
      }
    };



  }


}
