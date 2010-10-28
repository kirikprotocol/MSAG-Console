package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.web.components.data_table.model.DataTableRow;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public abstract class AggregatedStatRecord implements DataTableRow {

  private AggregationType type;
  private Calendar startCalendar;
  private Calendar endCalendar;
  private TreeMap<Date, AggregatedStatRecord> innerRowsMap = null;
  private List<AggregatedStatRecord> innerRows = null;
  private boolean isParent;
  private AggregationType childAggreagtionType;


  public AggregatedStatRecord(Calendar periodStart,  AggregationType type, boolean isParent) {    
    init(periodStart, type, isParent);
  }

  public AggregatedStatRecord(Date date, AggregationType type, boolean isParent) {
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    init(c,type, isParent);
  }


  private void init(Calendar periodStart, AggregationType type, boolean isParent) {

    startCalendar = (Calendar) periodStart.clone();
    startCalendar.set(Calendar.MILLISECOND,0);
    startCalendar.set(Calendar.SECOND,0);
    startCalendar.set(Calendar.MINUTE,0);

    this.type = type;
    this.isParent =isParent;
    childAggreagtionType = null;

    if(type== AggregationType.MONTH) {
      startCalendar.set(Calendar.DAY_OF_MONTH,1);
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.MONTH,1);
      if(isParent) childAggreagtionType = AggregationType.WEEK;
    }
    else if(type== AggregationType.WEEK) {

      if(startCalendar.get(Calendar.DAY_OF_WEEK)-Calendar.MONDAY<0) {
        startCalendar.add(Calendar.DATE,-7);
      }
      startCalendar.set(Calendar.DAY_OF_WEEK,Calendar.MONDAY);            
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE,7);
      if(isParent) childAggreagtionType = AggregationType.DAY;
    }
    else if(type== AggregationType.DAY) {
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE,1);
      if(isParent) childAggreagtionType = AggregationType.HOUR;
    }
    else if(type== AggregationType.HOUR) {
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.HOUR,1);
    }
    if(isParent) {
      innerRowsMap = new TreeMap<Date, AggregatedStatRecord>();
    }
  }


  protected void addChild(AggregatedStatRecord newChild) {
      Date d = newChild.getStartCalendar().getTime();
      AggregatedStatRecord oldChild = innerRowsMap.get(newChild.getStartCalendar().getTime());
      if(oldChild==null) {
        innerRowsMap.put(d,newChild);
      }
      else {
        oldChild.add(newChild);
      }
  }


  protected Map<Date, AggregatedStatRecord> getInnerRowsMap() {
    return innerRowsMap;
  }


  public abstract void add(AggregatedStatRecord other);

  public abstract  Comparator getRecordsComparator(final DataTableSortOrder sortOrder);
  

  protected void addChildren(AggregatedStatRecord other) {
    if(!isParent) return;
    for( AggregatedStatRecord newChild :   other.getInnerRowsMap().values()) {
      addChild(newChild);
    }
  }



  public AggregationType getType() {
    return type;
  }


  public Calendar getStartCalendar() {
    return startCalendar;
  }


  public Calendar getEndCalendar() {
    return endCalendar;
  }


  public String getPeriodString() {

    switch (type) {
      case HOUR :
        if(isParent) {
          return new SimpleDateFormat("yyyy.MM.dd HH:mm").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("HH:mm").format(endCalendar.getTime());
        }
        else {
              return new SimpleDateFormat("HH:mm").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("HH:mm").format(endCalendar.getTime());
        }

      case DAY :
        return new SimpleDateFormat("yyyy.MM.dd").format(startCalendar.getTime());

      case WEEK:
        Calendar endDateInclusive = (Calendar) endCalendar.clone();
        endDateInclusive.add(Calendar.MILLISECOND,-1);
        if(isParent) {
          return new SimpleDateFormat("yyyy.MM.dd").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("dd").format(endDateInclusive.getTime());

        }
        else {
          return new SimpleDateFormat("dd").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("dd").format(endDateInclusive.getTime());

        }

      default: //MONTH
        if(isParent) {
          return new SimpleDateFormat("yyyy.MM").format(startCalendar.getTime());
        }
        else {
          return new SimpleDateFormat("MM").format(startCalendar.getTime());
        }

    }
  }

  public String getId() {
    return null;
  }

  public Object getData() {
    return this;
  }

  public Object getInnerData() {
    return null;
  }

  public List getInnerRows() {
    if(!isParent) return null;
    if(innerRows==null) {
      innerRows = new ArrayList(innerRowsMap.values());
    }
    return innerRows;
  }

  public boolean isParent() {
    return isParent;
  }

  protected AggregationType getChildAggregationType() {
    return childAggreagtionType;
  }

  abstract void printCSV( PrintWriter writer);

  public void printWithChildrenToCSV( PrintWriter writer) {
    printCSV(writer);
    if(isParent() && getInnerRows()!=null) {
      for(AggregatedStatRecord child : (List<AggregatedStatRecord>)getInnerRows()) {
        child.printCSV(writer);
      }
    }
  }


}