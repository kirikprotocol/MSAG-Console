package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */

interface ErrorStatsStrategy {

  public void execute(Delivery delivery, User user, ProgressListener progressListener) throws Exception;

  public List getRows(int startPos, int count, final DataTableSortOrder sortOrder);

  public int getRowsCount();

  public void download(PrintWriter writer) throws IOException;

  static interface ProgressListener {
    void incrementCurrent();
    void setTotal(int t);
  }

}
