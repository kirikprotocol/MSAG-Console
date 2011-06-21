package mobi.eyeline.informer.web.components.data_table.model;

import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class EmptyDataTableModel implements DataTableModel {

  @Override
  public String getId(Object value) {
    return "";
  }

  public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
    return Collections.emptyList();
  }

  public int getRowsCount() {
    return 0;
  }
}
