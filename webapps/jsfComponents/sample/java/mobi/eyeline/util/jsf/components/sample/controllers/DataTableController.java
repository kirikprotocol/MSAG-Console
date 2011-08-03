package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.jsf.components.data_table.model.*;

import javax.faces.context.FacesContext;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class DataTableController {

  private String notification;

  private String field1Filter;

  public DataTableModel getModel() {

    final List<Row> rows = new LinkedList<Row>();
    for(int i=0;i<50;i++) {
      if(field1Filter != null && !Integer.toString(i).equals(field1Filter)) {
        continue;
      }
      rows.add(new Row(Integer.toString(i), Integer.toString(100+i)));
    }

    return new MyDataTableModel(rows);
  }

  public void setSelected(List selected) {
    System.out.println("Selected: "+selected);
  }

  public String choose() {
    String selected = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("selected");
    notification = "Selected: "+selected;
    return null;
  }

  public String getNotification() {
    return notification;
  }

  public void setNotification(String notification) {
    this.notification = notification;
  }

  public String getField1Filter() {
    return field1Filter;
  }

  public void setField1Filter(String field1Filter) {
    this.field1Filter = field1Filter == null || field1Filter.length() == 0 ? null : field1Filter;
  }

  private LoadListener loadListener;

  public void clear() {
    loadListener = null;
    loaded = false;
    field1Filter = null;
    notification = null;
  }

  private boolean loaded = false;

  public void query() {
    loadListener = null;
    loaded = false;
    notification = null;
  }

  public static class Row {

    private final List<Row> inner;

    private String field1;
    private String field2;

    public Row(String field1, String field2) {
      this.field1 = field1;
      this.field2 = field2;

      Random r = new Random();
      int size = r.nextInt(4);
      inner = new ArrayList<Row>(size);
      for(int i=0;i<size;i++) {
        inner.add(new Row(null, "f" + r.nextInt(100), "f" + r.nextInt(100)));
      }
    }

    public Row(List<Row> inner, String field1, String field2) {
      this.inner = inner;
      this.field1 = field1;
      this.field2 = field2;
    }

    public String getField1() {
      return field1;
    }

    public String getField2() {
      return field2;
    }

    public List<Row> getInner() {
      return inner;
    }

  }


  private class MyDataTableModel implements PreloadableModel, ModelWithObjectIds {

    private final List<Row> rows;

    public MyDataTableModel(List<Row> rows) {
      this.rows = rows;
    }

    public List getRows(int startPos, int count, final DataTableSortOrder dataTableSortOrder) {
      if(dataTableSortOrder == null || dataTableSortOrder.getColumnId().equals("field1")) {
        Collections.sort(rows, new Comparator<Row>() {
          public int compare(Row o1, Row o2) {
            return (dataTableSortOrder == null || dataTableSortOrder.isAsc() ? 1 : -1) * o1.getField1().compareTo(o2.field2);
          }
        });
      }else {
        Collections.sort(rows, new Comparator<Row>() {
          public int compare(Row o1, Row o2) {
            return (dataTableSortOrder.isAsc() ? 1 : -1) * o1.getField1().compareTo(o2.field2);
          }
        });
      }
      List<Row> result = new LinkedList<Row>();
      for (Iterator<Row> i = rows.iterator(); i.hasNext() && count > 0;) {
        Row r = i.next();
        if (--startPos < 0) {
          result.add(r);
          count--;
        }
      }
      return result;
    }


    public int getRowsCount() {
      return rows.size();
    }

    public String getId(Object o) {
      return ((Row)o).field1;
    }

    public LoadListener prepareRows(int startPos, int count, DataTableSortOrder sortOrder) throws ModelException {

//      if(System.currentTimeMillis()%20 == 0) {
//        throw new ModelException("interaction_error");
//      }
      LoadListener listener = null;
      if(!loaded) {
        if(loadListener == null) {
          loadListener = new LoadListener();
          new Thread() {
            public void run() {
              try{
                loadListener.setTotal(9);
                for(int i=0;i<10;i++) {
                  try {
                    Thread.sleep(250);
                    loadListener.setCurrent(i);
                  } catch (InterruptedException e) {}
                }
              }finally {
                loaded = true;
              }
            }
          }.start();
        }
        listener = loadListener;
      }
      return listener;
    }
  }

}
