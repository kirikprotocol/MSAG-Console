package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.page_calendar.PageCalendarModel;

import javax.faces.context.FacesContext;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class Index1Controller {

  private static int current = 0;

  private Date date;

  private List selected = new ArrayList(0);

  public String getSelectedStr() {
    return selected == null ? null : selected.toString();
  }

  public void setSelected(List selected) {
    System.out.println("Setter");
    this.selected = selected;
  }

  public int[][] getValues() {
    Random r = new Random();
    int[][] res = new int[10][2];
    for(int i=0;i<10;i++) {
      res[i][0] = i;
      res[i][1] = r.nextInt(100);
    }
    return res;
  }

  public List getLabels() {
    Random r = new Random();
    List res = new ArrayList(10);
    for(int i=0;i<10;i++) {
      res.add("label"+r.nextInt(10));
    }
    return res;
  }

  public Date getDate() {
    return date;
  }

  public void setDate(Date date) {
    this.date = date;
  }

  public String printDate() {
    date = new Date();
    if(selected != null) {
//      selected.clear();
    }
    System.out.println(date);
    current = 0;
    return null;
  }

  public String print() {
    System.out.println("Print");
    return null;
  }

  public int getMaximum() {
    return 100;
  }

  public boolean isContinueUpdate() {
    return current < 10;
  }


  public int getCurrent() {
    for (Map.Entry e : FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().entrySet())
      System.out.println(e.getKey() + " " + e.getValue());

    if (current < 1000) {
      current++;
    }
    return current;
  }

  final SimpleDateFormat df = new SimpleDateFormat("dd-MM-yyyy");

  public PageCalendarModel getCalendarModel() {
    return new PageCalendarModel() {

      public void updateVisiblePeriod(Date startDate, Date endDate) {
        System.out.println("Visible period: " + df.format(startDate) + " - " + df.format(endDate));
      }

      public Object getValue(Date date) {
        return df.format(date);
      }
    };
  }


  public  DataTableModel getModel() {

    final ArrayList<Integer> list = new ArrayList<Integer>();

    for (int i = 0; i < 100; i++)
      list.add(i);

    return new MyDataTableModel(list);
  }

  public String clear() {
    current = 0;
    return null;
  }

  public static class MyDataTableModel implements DataTableModel {

    private final ArrayList<Integer> list;

    public MyDataTableModel(ArrayList<Integer> list) {
      this.list = list;
    }

    public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
      if (sortOrder != null && sortOrder.getColumnId().equals("mycolumn")) {
        Collections.sort(list, new Comparator<Integer>() {

          public int compare(Integer o1, Integer o2) {
            if (sortOrder.isAsc())
              return o1.compareTo(o2);
            else
              return -o1.compareTo(o2);
          }
        });
      }

      ArrayList<Integer> res = new ArrayList<Integer>();
      for (int i = startPos; i < Math.min(startPos + count, list.size()); i++)
        res.add(list.get(i));

      return res;
    }

    public int getRowsCount() {
      return list.size();
    }

    public String getId(Object value) {
      return value != null ? value.toString() : null;
    }

  }
}
