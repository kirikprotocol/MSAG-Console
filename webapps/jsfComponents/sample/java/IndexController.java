import mobi.eyeline.util.Time;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.util.jsf.components.page_calendar.PageCalendarModel;
import org.apache.myfaces.renderkit.html.HtmlRenderKitImpl;

import javax.faces.render.RenderKit;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class IndexController {

  private DynamicTableModel dynamicTableModel;

  private Date inputDate;

  private int nextInt = new Random().nextInt(100);

  public int getNextInt() {
    return nextInt;
  }

  public String getInputDateString() {
    return inputDate == null ? null : inputDate.toString();
  }

  public Date getInputDate() {
    return inputDate;
  }

  public void setInputDate(Date inputDate) {
    this.inputDate = inputDate;
  }



  private Time time;


  public Time getTime() {
    return time;
  }

  public void setTime(Time time) {
    this.time = time;
  }

  public String getTimeString() {
    return time == null ? null : time.toString();
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

  public IndexController() {
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

  public DataTableModel getModel() {

    final List<Row> rows = new ArrayList<Row>(50);
    for(int i=0;i<50;i++) {
      rows.add(new Row(Integer.toString(i), Integer.toString(100+i)));
    }

    return new DataTableModel() {

      public List getRows(int i, int i1, final DataTableSortOrder dataTableSortOrder) {

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

        return rows.subList(i, i+i1 > rows.size() ? rows.size() : i+i1);
      }

      public int getRowsCount() {
        return 50;
      }
    };
  }

  public void setSelected(List selected) {
    System.out.println("Selected: "+selected);
  }


  public void clear() {


  }

  public void query() {

  }

  public static class Row {

    private String field1;
    private String field2;

    public Row(String field1, String field2) {
      this.field1 = field1;
      this.field2 = field2;
    }

    public String getField1() {
      return field1;
    }

    public String getField2() {
      return field2;
    }
  }

  public String getDate() {
    return new Date().toString();
  }


  public PageCalendarModel getCalendarModel() {

    return new PageCalendarModel() {  RenderKit k = new HtmlRenderKitImpl();

      public void updateVisiblePeriod(Date startDate, Date endDate) {
      }

      public Object getValue(Date date) {
        return date.toString();
      }
    };
  }



}
