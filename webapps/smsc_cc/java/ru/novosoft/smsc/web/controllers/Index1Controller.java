package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.web.components.dynamic_table.TableModel;
import ru.novosoft.smsc.web.components.dynamic_table.TableRow;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Index1Controller implements Serializable {

  transient private TableModel model;

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
//    System.out.println("NEW MODEL");
//    for (TableRow row : model.getRows())
//      System.out.println(row.getValue("column1") + " | " + row.getValue("column2"));
    this.model = model;
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

  public String getLabel() {
    return "MyLabel";
  }
}
