package mobi.eyeline.util.jsf.components.data_table.model;

import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class EmptyDataTableModel implements DataTableModel, ModelWithObjectIds {
  public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
    return Collections.emptyList();
  }

  public int getRowsCount() {
    return 0;
  }

  public String getId(Object o) {
    return null;
  }
}
