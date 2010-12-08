package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.StatEntity;
import mobi.eyeline.informer.admin.delivery.stat.StatEntityProvider;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.notifications.DateAndFile;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.text.ParseException;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 24.11.2010
 * Time: 11:19:29
 */
public class StatsSizeController extends DeliveryStatController  {
  private List<String> selectedRows;


  public StatsSizeController() {
    super(new StatsSizeTotals());
    setAggregation(TimeAggregationType.MONTH);
  }




  @Override
  void loadRecords(Configuration config, Locale locale) throws AdminException, InterruptedException {

    DeliveryStatFilter filter = getFilter();
    final List<StatEntity> files = new LinkedList<StatEntity>();
    config.getStatEntities(new StatEntityProvider.EntityVisitor() {
      @Override
      public boolean visit(StatEntity entity) throws AdminException {
        files.add(entity);
        return true;
      }
    }, filter.getFromDate(), filter.getTillDate()
    );
    setCurrentAndTotal(0, files.size()==0 ? 1:files.size());

    for(StatEntity f : files) {

      Calendar c = Calendar.getInstance();
      c.setTime(f.getDate());
      AggregatedRecord newRecord = new StatsSizeRecord(c, getAggregation(), true,1,f.getSize());
      AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
      if (oldRecord == null) {
        putRecord(newRecord);
      } else {
        oldRecord.add(newRecord);
      }
      getTotals().add(newRecord);
      setCurrent(getCurrent() + 1);

    }

    FileSystem fileSys = config.getFileSystem();
    List<DateAndFile> dfiles = getConfig().getProcessedNotificationsFiles(getFilter().getFromDate(),getFilter().getTillDate());
    for(DateAndFile df : dfiles) {
      AggregatedRecord newRecord = new StatsSizeRecord(df.getCalendar(), getAggregation(), true,1,fileSys.length(df.getFile()));
      AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
      if (oldRecord == null) {
        putRecord(newRecord);
      } else {
        oldRecord.add(newRecord);
      }
      getTotals().add(newRecord);
      setCurrent(getCurrent() + 1);
    }
  }

  @Override
  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(TimeAggregationType.DAY));
    ret.add(new SelectItem(TimeAggregationType.WEEK));
    ret.add(new SelectItem(TimeAggregationType.MONTH));
    ret.add(new SelectItem(TimeAggregationType.YEAR));
    return ret;
  }

  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }

  public String removeSelected() {
    if(selectedRows == null || selectedRows.isEmpty()) {
      return null;
    }
    if (logger.isDebugEnabled())
      logger.debug("Start remove statistics.");

    FileSystem fileSys = getConfig().getFileSystem();
    DeliveryStatFilter filter = new DeliveryStatFilter();
    boolean addAttention = false;
    Date now = new Date();
    for(String s : selectedRows) {
      try {
        String[] pair = s.split("-");
        if (logger.isDebugEnabled())
          logger.debug("Removing statistics for period: " + s);
        Date from = StatsSizeRecord.getPeriodIdFormat().parse(pair[0]);
        Date till = StatsSizeRecord.getPeriodIdFormat().parse(pair[1]);
        if(till == null || (now.compareTo(till) < 0 && (from == null || from.compareTo(now) <= 0))) {
          addAttention = true;
        }
        getConfig().dropStatEntities(from, till);
        List<DateAndFile> dfiles = getConfig().getProcessedNotificationsFiles(filter.getFromDate(),filter.getTillDate());
        for(DateAndFile f : dfiles) {
          try {
            if (logger.isDebugEnabled())
              logger.debug("Remove file: " + f.getFile());
            fileSys.delete(f.getFile());
          }
          catch (AdminException e) {
            addError(e);
          }
        }
      }
      catch (ParseException e) {
        //never should happen
        addMessage(FacesMessage.SEVERITY_ERROR,"internal.error");
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    start();
    if(addAttention) {
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "stat.clean.today.attention");
    }

    if (logger.isDebugEnabled())
      logger.debug("Finish remove statistics.");
    return null;
  }

  @Override
  public String start() {
    selectedRows = null;
    return super.start();
  }
}
