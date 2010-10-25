package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.web.components.data_table.model.DataTableRow;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public class AggregatedDeliveryStatRecord implements DataTableRow {

  private AggregationType type;
  private Calendar startCalendar;
  private Calendar endCalendar;
  private int delivered;
  private int failed;
  private List<AggregatedDeliveryStatRecord> details;
  private boolean isParent;

  public AggregatedDeliveryStatRecord(DeliveryStatRecord dsr, AggregationType type, boolean isParent) {
    this.type = type;
    this.isParent =isParent;
    startCalendar = Calendar.getInstance();
    startCalendar.clear();

    startCalendar.set(dsr.getYear(),dsr.getMonth()-1,dsr.getDay(),dsr.getHour(),dsr.getMinute());

    if(type== AggregationType.MONTH) {
      startCalendar.set(Calendar.DAY_OF_MONTH,1);
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.MONTH,1);
      if(isParent) createDetails(dsr, AggregationType.WEEK);
    }
    if(type== AggregationType.WEEK) {
      startCalendar.set(Calendar.DAY_OF_WEEK,0);
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE,7);
      if(isParent) createDetails(dsr, AggregationType.DAY);
    }
    if(type== AggregationType.DAY) {
      startCalendar.set(Calendar.HOUR_OF_DAY,0);
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.DATE,1);
      if(isParent) createDetails(dsr, AggregationType.HOUR);
    }
    if(type== AggregationType.HOUR) {
      startCalendar.set(Calendar.MINUTE,0);
      endCalendar = (Calendar) startCalendar.clone();
      endCalendar.add(Calendar.HOUR,1);
      //nothing to set;
    }


    this.delivered = dsr.getDelivered();
    this.failed = dsr.getFailed();
  }

  private void createDetails(DeliveryStatRecord dsr, AggregationType type) {
    this.details = new ArrayList<AggregatedDeliveryStatRecord>();
    details.add(new AggregatedDeliveryStatRecord(dsr,type,false));
  }


  boolean add(AggregatedDeliveryStatRecord other) {
    if(other.type!=this.type || other.isParent !=this.isParent) {
      throw new IllegalArgumentException("Can't add different types of aggregated stat");
    }
    if(startCalendar.compareTo(other.startCalendar)==0) {
      if(endCalendar.compareTo(other.endCalendar)==0) {
        this.delivered += other.delivered;
        this.failed += other.failed;
        if(this.isParent && type!=AggregationType.HOUR) {

outer:  for(AggregatedDeliveryStatRecord otherchild : other.getDetails()) {
            for(AggregatedDeliveryStatRecord thisChild : getDetails()) {
              if(thisChild.add(otherchild)) {
                continue outer;
              }
            }
            getDetails().add(otherchild);
          }
        }
        return true;
      }
    }
    return false;
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


  public int getDelivered() {
    return delivered;
  }


  public int getFailed() {
    return failed;
  }


  public List<AggregatedDeliveryStatRecord> getDetails() {
    return details;
  }

  public String getPeriodString() {
    SimpleDateFormat fmt1;
    SimpleDateFormat fmt2;
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
        if(isParent) {
          return new SimpleDateFormat("yyyy.MM.dd").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("dd").format(endCalendar.getTime());

        }
        else {
          return new SimpleDateFormat("dd").format(startCalendar.getTime())
              +  " - "
              +  new SimpleDateFormat("dd").format(endCalendar.getTime());

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
    return null;  //To change body of implemented methods use File | Settings | File Templates.
  }

  public Object getData() {
    return this;
  }

  public Object getInnerData() {
    return null;
  }

  public List<Object> getInnerRows() {
    return isParent ? (List)details : null;
  }

  public boolean isParent() {
    return isParent;
  }

  enum AggregationType {
    HOUR,DAY,WEEK,MONTH
  }
}
