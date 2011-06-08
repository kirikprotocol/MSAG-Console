package mobi.eyeline.util.jsf.components.dynamic_table.model;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DynamicTableModel implements Serializable {

  private List<DynamicTableRow> rows = new ArrayList<DynamicTableRow>();

  public void addRow(DynamicTableRow row) {
    rows.add(row);
  }

  public List<DynamicTableRow> getRows() {
    return rows;
  }

  public int getRowCount() {
    return rows.size();
  }
}
