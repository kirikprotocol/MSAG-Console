package mobi.eyeline.informer.web.components.data_table.model;

import mobi.eyeline.informer.web.components.data_table.Identificator;

import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class EmptyDataTableModel implements DataTableModel, Identificator {

  public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
    return Collections.emptyList();
  }

  public int getRowsCount() {
    return 0;
  }

  @Override
  public String getId(Object o) {
    return null;
  }
}
