package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class Index1Controller {

  private static int current=0;

  public int getMaximum() {
    return 100;
  }

  public boolean isContinueUpdate() {
    return current < 10;
  }


  public int getCurrent() {
    if(current < 1000) {
      current++;
    }
    return current;
  }

  public DataTableModel getModel() {

    System.out.println("GET MODEL CALLED");

    final ArrayList<Integer> list = new ArrayList<Integer>();

    for (int i=0; i<100; i++)
      list.add(i);

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        System.out.println("START: " + startPos);
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
        for (int i=startPos; i<Math.min(startPos + count, list.size()); i++)
          res.add(list.get(i));

        return res;
      }

      public int getRowsCount() {
        return list.size();
      }
    };
  }

  public String clear() {
    current = 0;
    return null;
  }
}
