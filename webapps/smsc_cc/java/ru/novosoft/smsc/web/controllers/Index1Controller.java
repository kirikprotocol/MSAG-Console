package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRowBase;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;

import javax.faces.event.ActionEvent;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Index1Controller implements Serializable {

  private boolean init=false;
  

  public void query() {
    init = true;
  }


  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  transient private DynamicTableModel model;
  static int counter;

  public Index1Controller() {
    model = new DynamicTableModel();
  }

  public DynamicTableModel getModel() {
    return model;
  }

  public void setModel(DynamicTableModel model) {
    for (DynamicTableRow row : model.getRows())
      System.out.println(row.getValue("column1") + " | " + row.getValue("column2"));
    this.model = model;
  }


  public DataTableModel getStaticModel() {
    return new MyPagedTableModel();
  }

  public void setSelectedRows(List rows) {
    System.out.println("SET SELECTED ROWS:");
    for (Object o : rows)
      System.out.println("  " + o);
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

  private class MyPagedTableModel implements DataTableModel {

    private final List<String> strings = new ArrayList<String>();

    public MyPagedTableModel() {
      for (int i = 0; i < 100; i++)
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
        RowData d = new RowData(strings.get(i), strings.get(i) + "2");

        DataTableRowBase rb = new DataTableRowBase(strings.get(i), d, i % 3 == 2 ? ("INNER DATA " + strings.get(i)) : null);

        if (i % 3 == 1) {
          rb.addInnerRow(d);
          rb.addInnerRow(d);
          rb.addInnerRow(d);
          rb.addInnerRow(d);
          rb.addInnerRow(d);

        }

        rows.add(rb);
      }
      return rows;
    }

    public int getRowsCount() {
      return strings.size();
    }

    public class RowData {
      private final String column1;
      private final String column2;

      public RowData(String column1, String column2) {
        this.column1 = column1;
        this.column2 = column2;
      }

      public String getColumn1() {
        return column1;
      }

      public String getColumn2() {
        return column2;
      }
    }

  }
}
