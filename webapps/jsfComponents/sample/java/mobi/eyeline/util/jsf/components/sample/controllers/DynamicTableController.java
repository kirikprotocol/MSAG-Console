package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;

import java.util.ArrayList;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class DynamicTableController {

  private DynamicTableModel dynamicTableModel;

  public DynamicTableController() {
    dynamicTableModel = new DynamicTableModel();
    DynamicTableRow row = new DynamicTableRow();
    row.setValue("field1", "1");
    row.setValue("field2", "11");
    dynamicTableModel.addRow(row);
    row = new DynamicTableRow();
    row.setValue("field1", "2");
    row.setValue("field2", "22");
    dynamicTableModel.addRow(row);
  }

  public DynamicTableModel getDynamicTableModel() {
    return dynamicTableModel;
  }

  public void setDynamicTableModel(DynamicTableModel dynamicTableModel) {
    this.dynamicTableModel = dynamicTableModel;
  }


  public List<Row> getRows() {
    List<Row> result = new ArrayList<Row>(dynamicTableModel.getRowCount());
    for(DynamicTableRow r : dynamicTableModel.getRows()) {
      result.add(new Row(r.getValue("field1"), r.getValue("field2")));
    }
    return result;
  }

  public static class Row {
    private Object field1;
    private Object field2;
    public Row(Object field1, Object field2) {
      this.field1 = field1;
      this.field2 = field2;
    }
    public Object getField1() {
      return field1;
    }
    public Object getField2() {
      return field2;
    }
  }



}
