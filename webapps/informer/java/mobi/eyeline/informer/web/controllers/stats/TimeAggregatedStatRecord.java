package mobi.eyeline.informer.web.controllers.stats;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.TreeMap;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public abstract class TimeAggregatedStatRecord extends AggregatedRecord {

  private TimeAggregationType type;
  protected Calendar startCalendar;
  protected Calendar endCalendar;
  private TimeAggregationType childAggreagtionType;


  public TimeAggregatedStatRecord(Calendar periodStart, TimeAggregationType type, boolean isParent) {
    init(periodStart, type, isParent);
  }

  public TimeAggregatedStatRecord(Date date, TimeAggregationType type, boolean isParent) {
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    init(c, type, isParent);
  }


  private void init(Calendar periodStart, TimeAggregationType type, boolean isParent) {

    startCalendar = (Calendar) periodStart.clone();
    startCalendar.set(Calendar.MILLISECOND, 0);
    startCalendar.set(Calendar.SECOND, 0);
    startCalendar.set(Calendar.MINUTE, 0);

    this.type = type;
    this.isParent = isParent;
    childAggreagtionType = null;
    if (type == TimeAggregationType.YEAR) {
      startCalendar.set(Calendar.MONTH, 0);
      startCalendar.set(Calendar.DAY_OF_MONTH, 1);
      startCalendar.set(Calendar.HOUR_OF_DAY, 0);
      startCalendar.set(Calendar.MINUTE, 0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.YEAR, 1);
      if (isParent) childAggreagtionType = TimeAggregationType.MONTH;
    } else  if (type == TimeAggregationType.MONTH) {
      startCalendar.set(Calendar.DAY_OF_MONTH, 1);
      startCalendar.set(Calendar.HOUR_OF_DAY, 0);
      startCalendar.set(Calendar.MINUTE, 0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.MONTH, 1);
      if (isParent) childAggreagtionType = TimeAggregationType.WEEK;
    } else if (type == TimeAggregationType.WEEK) {

      if (startCalendar.get(Calendar.DAY_OF_WEEK) - Calendar.MONDAY < 0) {
        startCalendar.add(Calendar.DATE, -7);
      }
      startCalendar.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);
      startCalendar.set(Calendar.HOUR_OF_DAY, 0);
      startCalendar.set(Calendar.MINUTE, 0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE, 7);
      if (isParent) childAggreagtionType = TimeAggregationType.DAY;
    } else if (type == TimeAggregationType.DAY) {
      startCalendar.set(Calendar.HOUR_OF_DAY, 0);
      startCalendar.set(Calendar.MINUTE, 0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE, 1);
      if (isParent) childAggreagtionType = TimeAggregationType.HOUR;
    } else if (type == TimeAggregationType.HOUR) {
      startCalendar.set(Calendar.MINUTE, 0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.HOUR, 1);
    }
    if (isParent) {
      innerRowsMap = new TreeMap<Object, AggregatedRecord>();
    }
  }


  public Object getAggregationKey() {
    return getStartCalendar().getTime();
  }


  public TimeAggregationType getType() {
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
      case HOUR:
        if (isParent) {
          return new SimpleDateFormat("yyyy.MM.dd HH:mm").format(startCalendar.getTime())
              + " - "
              + new SimpleDateFormat("HH:mm").format(endCalendar.getTime());
        } else {
          return new SimpleDateFormat("HH:mm").format(startCalendar.getTime())
              + " - "
              + new SimpleDateFormat("HH:mm").format(endCalendar.getTime());
        }

      case DAY:
        return new SimpleDateFormat("yyyy.MM.dd").format(startCalendar.getTime());

      case WEEK:
        Calendar endDateInclusive = (Calendar) endCalendar.clone();
        endDateInclusive.add(Calendar.MILLISECOND, -1);
        if (isParent) {
          return new SimpleDateFormat("yyyy.MM.dd").format(startCalendar.getTime())
              + " - "
              + new SimpleDateFormat("dd").format(endDateInclusive.getTime());

        } else {
          return new SimpleDateFormat("dd").format(startCalendar.getTime())
              + " - "
              + new SimpleDateFormat("dd").format(endDateInclusive.getTime());

        }

      case MONTH:
        if (isParent) {
          return new SimpleDateFormat("yyyy.MM").format(startCalendar.getTime());
        } else {
          return new SimpleDateFormat("MM").format(startCalendar.getTime());
        }

      default: // YEAR:
        if (isParent) {
          return new SimpleDateFormat("yyyy").format(startCalendar.getTime());
        } else {
          return new SimpleDateFormat("yyyy").format(startCalendar.getTime());
        }

      

    }
  }

  protected TimeAggregationType getChildAggregationType() {
    return childAggreagtionType;
  }

  abstract void printCSVheader(PrintWriter writer, boolean detalized);

  abstract void printCSV(PrintWriter writer, boolean detalized);


  public void printWithChildrenToCSV(PrintWriter writer, boolean detalized) {
    printCSV(writer, detalized);
    if (isParent() && getInnerRows() != null) {
      for (TimeAggregatedStatRecord child : (List<TimeAggregatedStatRecord>) getInnerRows()) {
        child.printCSV(writer, detalized);
      }
    }
  }


}