package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.notifications.DateAndFile;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.io.File;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;

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
    FileSystem fileSys = getConfig().getFileSystem();
    List<File> files = getConfig().getStatisticFiles(getFilter(),false);
    setCurrentAndTotal(0, files.size()==0 ? 1:files.size());
    for(File f : files) {

        Calendar c = getConfig().getCalendarOfStatFile(f);

        AggregatedRecord newRecord = new StatsSizeRecord(c, getAggregation(), true,1,fileSys.length(f));
        AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
        if (oldRecord == null) {
          putRecord(newRecord);
        } else {
          oldRecord.add(newRecord);
        }
        getTotals().add(newRecord);
        setCurrent(getCurrent() + 1);

    }
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
    if (logger.isDebugEnabled())
      logger.debug("Start remove statistics.");

    FileSystem fileSys = getConfig().getFileSystem();
    DeliveryStatFilter filter = new DeliveryStatFilter();
    for(String s : selectedRows) {
      try {
        String[] pair = s.split("-");
        if (logger.isDebugEnabled())
          logger.debug("Removing statistict for period: " + s);
        filter.setFromDate(StatsSizeRecord.getPeriodIdFormat().parse(pair[0]));
        filter.setTillDate(StatsSizeRecord.getPeriodIdFormat().parse(pair[1]));
        List<File> files = getConfig().getStatisticFiles(filter,false);
        for(File f : files) {
          try {
            if (logger.isDebugEnabled())
              logger.debug("Remove file: " + f);
            fileSys.delete(f);
          }
          catch (AdminException e) {
            addError(e);
          }
        }
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

    if (logger.isDebugEnabled())
      logger.debug("Finish remove statistics.");
    return null;
  }
}
