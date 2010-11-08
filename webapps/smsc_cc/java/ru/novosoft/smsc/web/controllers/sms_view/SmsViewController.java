package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.*;

/**
 * @author Artem Snopkov
 */
public class SmsViewController extends SmscController {

  private static final ExecutorService tpool = Executors.newCachedThreadPool();

  private int storageType;
  private SmsQuery query = new SmsQuery();
  private Future<DataTableModel> future;
  private GetSmsProgress progress = new GetSmsProgress();
  private WebContext wcontext = WebContext.getInstance();

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

  public boolean isArchiveDaemonDeployed() {
    return wcontext.getArchiveDaemonManager() != null;
  }

  public DataTableModel getResults() {
    if (getState() == 2) {
      try {
        return future.get();
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
