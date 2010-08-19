package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.web.components.data_table.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.DataTableSortOrder;
import ru.novosoft.smsc.web.components.dynamic_table.TableModel;
import ru.novosoft.smsc.web.components.dynamic_table.TableRow;

import javax.faces.event.ActionEvent;
import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class Index1Controller implements Serializable {

  transient private TableModel model;
  static int counter;

  public Index1Controller() {
    model = new TableModel();
//    for (int i=0; i<10; i++) {
//      TableRow row = new TableRow();
//      row.setValue("column1", "value1-" + i);
//      row.setValue("column2", "value2-" + i);
//      row.setValue("column3", "value" + (i%3 + 1));
//      row.setValue("column4", "value" + (i%3 + 4));
//      model.addRow(row);
//    }
  }

  public TableModel getModel() {
    return model;
  }

  public void setModel(TableModel model) {
    System.out.println("SET MODEL CALLED");
    for (TableRow row : model.getRows())
      System.out.println(row.getValue("column1") + " | " + row.getValue("column2"));
    this.model = model;
  }

  public String getColor() {
    return "GREEN";
  }

  public DataTableModel getStaticModel() {
    return new MyPagedTableModel();
  }

  public List<String> getColumn3Values() {
    List<String> res = new ArrayList<String>();
    Collections.addAll(res, "value1", "value2", "value3");
    return res;
  }

  public List<String> getColumn4Values() {
    List<String> res = new ArrayList<String>();
    Collections.addAll(res, "value4", "value5", "value6");
    return res;
  }

  public boolean isOpened() {
    return true;
  }

  public void setFilter(String filter) {
    System.out.println("SET FILTER " + filter);
  }

  public String getFilter() {
    return "";
  }

  public void myButton(ActionEvent e) {
    System.out.println("MY BUTTON CALLED");
  }

  public void edit(ActionEvent e) {
    System.out.println("EDIT CALLED");
  }

  public String getLabel() {
    return "MyLabel";
  }

  public int getCounter() {
    return counter++;
  }

  private class MyPagedTableModel extends DataTableModel {

    private final List<String> strings = new ArrayList<String>();

    public MyPagedTableModel() {
      for (int i = 0; i < 30; i++)
        strings.add(String.valueOf(i));
    }

    public List<DataTableRow> getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
      if (sortOrder != null) {
        Collections.sort(strings, new Comparator<String>() {
          public int compare(String o1, String o2) {
            if (sortOrder.isAsc())
              return o1.compareTo(o2);
            else
              return -o1.compareTo(o2);
          }
        });
      }
      List<DataTableRow> rows = new ArrayList<DataTableRow>();
      for (int i = startPos; i < startPos + count && i < strings.size(); i++) {
        Map<String, Object> val = new HashMap<String, Object>();
        val.put("column1", strings.get(i));
        val.put("column2", strings.get(i) + "2");
        val.put("column3", i%2);
        rows.add(new Row(val));
      }
      return rows;
    }

    public void setSelectedRows(int[] rows) {
      if (rows != null) {
        System.out.println("SELECTED ROWS: ");
        for (int i = 0; i < rows.length; i++)
          System.out.println(rows[i]);
      }
    }

    public int getRowsCount() {
      return strings.size();
    }

    private class Row implements DataTableRow {

      private final Map<String, Object> values;

      private Row(Map<String, Object> values) {
        this.values = values;
      }

      public Object getData(String columnId) {
        return values.get(columnId);
      }

      public Object getInnerData() {
        return null;  //To change body of implemented methods use File | Settings | File Templates.
      }
    }
  }
}
