package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.*;

/**
 * @author Artem Snopkov
 */
public class SmsViewController extends SmscController {

  private static final ExecutorService tpool = Executors.newCachedThreadPool();
  private static final String SMS_VIEW_FUTURE = "smsViewFuture";

  private int storageType;
  private SmsQuery query = new SmsQuery();
  private Future<DataTableModel> future;
  private GetSmsProgress progress = new GetSmsProgress();
  private WebContext wcontext = WebContext.getInstance();
  private SmsStatus smsStatus;
  private boolean queryExact;

  public SmsViewController() {
    future = (Future)getSession(true).getAttribute(SMS_VIEW_FUTURE);
  }

  public int getState() {
    if (future == null)
      return 0;
    if (!future.isDone())
      return 1;
    return 2;
  }

  public SmsQuery getQuery() {
    return query;
  }

  public GetSmsProgress getProgress() {
    return progress;
  }

  public int getStorageType() {
    return storageType;
  }

  public void setStorageType(int storageType) {
    this.storageType = storageType;
  }

  public SmsStatus getSmsStatus() {
    return smsStatus;
  }

  public void setSmsStatus(SmsStatus smsStatus) {
    this.smsStatus = smsStatus;
  }

  public List<SelectItem> getSmsStatuses() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    for (SmsStatus s : SmsStatus.values())
      res.add(new SelectItem(s));
    return res;
  }

  public List<SelectItem> getMaxRows() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    Collections.addAll(res,
        new SelectItem(null, ""),
        new SelectItem(100),
        new SelectItem(200),
        new SelectItem(300),
        new SelectItem(400),
        new SelectItem(500),
        new SelectItem(1000),
        new SelectItem(2000),
        new SelectItem(5000)
        );
    return res;
  }

  public boolean isQueryExact() {
    return queryExact;
  }

  public void setQueryExact(boolean queryExact) {
    this.queryExact = queryExact;
  }

  public boolean isArchiveDaemonDeployed() {
    return wcontext.getArchiveDaemonManager() != null;
  }

  public DataTableModel getResults() {
    if (getState() == 2) {
      try {
        DataTableModel result = future.get();
        cancel();
        return result;
      } catch (InterruptedException e) {
      } catch (ExecutionException e) {
        Throwable cause = e.getCause();
        if (cause instanceof AdminException)
          addError((AdminException) cause);
        else
          addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage());
      }
    }

    return new DataTableModel() {
      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        return Collections.emptyList();
      }

      public int getRowsCount() {
        return 0;
      }
    };
  }


  public synchronized String applyFilter() {
    cancel();

    try {
      GetSmsStrategy strategy ;
      if (storageType == 1) {
        strategy = new GetSmsFromOperative(wcontext.getOperativeStoreProvider(), wcontext.getSmscManager().getSettings().getSmscInstancesCount());
      } else {
        strategy = new GetSmsFromArchive(wcontext.getArchiveDaemon());
      }

      future = tpool.submit(new GetSmsTask(strategy, query, progress));
      getSession(true).setAttribute(SMS_VIEW_FUTURE, future);
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public synchronized String cancel() {
    if (future != null) {
      if (!future.isDone())
        future.cancel(true);
      future = null;
      getSession(true).removeAttribute(SMS_VIEW_FUTURE);
    }
    return null;
  }

  public String clearFilter() {
    cancel();
    query.clear();
    return null;
  }

  private static class GetSmsTask implements Callable<DataTableModel> {

    private GetSmsStrategy strategy;
    private SmsQuery query;
    private GetSmsProgress progress;

    private GetSmsTask(GetSmsStrategy strategy, SmsQuery query, GetSmsProgress progress) {
      this.strategy = strategy;
      this.query = query;
      this.progress = progress;
    }

    public DataTableModel call() throws Exception {
      return strategy.getSms(query, progress);
    }
  }
}
