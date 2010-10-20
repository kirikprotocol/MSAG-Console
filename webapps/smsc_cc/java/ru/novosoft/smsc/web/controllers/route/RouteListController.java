package ru.novosoft.smsc.web.controllers.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Destination;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Source;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class RouteListController extends RouteController {

  private String nameFilter;
  private String srcSmeFilter;
  private String dstSmeFilter;
  private String subjectFilter;
  private String maskFilter;

  private List<String> routes2remove;


  public RouteListController() {
  }

  public void clearFilter() {
    nameFilter = null;
    srcSmeFilter = null;
    dstSmeFilter = null;
    subjectFilter = null;
    maskFilter = null;
  }

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter;
  }

  public String getSrcSmeFilter() {
    return srcSmeFilter;
  }

  public void setSrcSmeFilter(String srcSmeFilter) {
    this.srcSmeFilter = srcSmeFilter;
  }

  public String getDstSmeFilter() {
    return dstSmeFilter;
  }

  public void setDstSmeFilter(String dstSmeFilter) {
    this.dstSmeFilter = dstSmeFilter;
  }

  public String getSubjectFilter() {
    return subjectFilter;
  }

  public void setSubjectFilter(String subjectFilter) {
    this.subjectFilter = subjectFilter;
  }

  public String getMaskFilter() {
    return maskFilter;
  }

  public void setMaskFilter(String maskFilter) {
    this.maskFilter = maskFilter;
  }

  private static boolean isEmpty(String str) {
    return str == null || str.trim().length() == 0;
  }

  public void setRoutes2remove(List<String> routes2remove) {
    this.routes2remove = routes2remove;
  }

  /**
   * Возвращает список маршрутов, удовлетвояющих всем фильтрам
   * @return  список маршрутов, удовлетвояющих всем фильтрам
   */
  private List<Route> filterRoutes() {
    RouteSubjectSettings s = getSettings();

    final List<Route> routes = s.getRoutes();

    for (Iterator<Route> iter =  routes.iterator(); iter.hasNext();) {
      Route r = iter.next();

      boolean toRemove = !isEmpty(nameFilter) && !r.getName().startsWith(nameFilter);

      toRemove = toRemove || (!isEmpty(srcSmeFilter) && (r.getSrcSmeId() == null || r.getSrcSmeId().equals(srcSmeFilter)));

      if (!isEmpty(subjectFilter) || !isEmpty(maskFilter)) {
        boolean found = false;
        for (Source src : r.getSources()) {
          found = !isEmpty(subjectFilter) && src.getSubject() != null && src.getSubject().equals(subjectFilter);
          found = found || !isEmpty(maskFilter) && src.getMask() != null && src.getMask().getSimpleAddress().equals(maskFilter);
          if (found) {
            found = true;
            break;
          }
        }
        toRemove = toRemove || !found;
      }

      if (!isEmpty(dstSmeFilter) || !isEmpty(subjectFilter) || !isEmpty(maskFilter)) {
        boolean found = false;
        for (Destination src : r.getDestinations()) {
          found = !isEmpty(subjectFilter) && src.getSubject() != null && src.getSubject().equals(subjectFilter);
          found = found || !isEmpty(maskFilter) && src.getMask() != null && src.getMask().getSimpleAddress().equals(maskFilter);
          found = found || !isEmpty(dstSmeFilter) && src.getSmeId() != null && src.getSmeId().equals(dstSmeFilter);
          if (found) {
            found = true;
            break;
          }
        }
        toRemove = toRemove || !found;
      }

      if (toRemove)
        iter.remove();
    }

    return routes;
  }

  public String removeRoutes() {
    RouteSubjectSettings s = getSettings();
    List<Route> routes = s.getRoutes();
    if (routes2remove != null) {
      for (String route : routes2remove) {
        for (Iterator<Route> iter = routes.iterator(); iter.hasNext();) {
          if (iter.next().getName().equals(route))
            iter.remove();
        }
      }

      try {
        s.setRoutes(routes);
        setSettings(s);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String submit() {
    try {
      submitSettings();
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "ROUTES";
  }

  public DataTableModel getRoutes() {
    final List<Route> routes = filterRoutes();

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        if (sortOrder != null) {
          Collections.sort(routes, new Comparator<Route>() {
            public int compare(Route o1, Route o2) {
              int val = 0;
              if (sortOrder.getColumnId().equals("name"))
                val = o1.getName().compareTo(o2.getName());
              return sortOrder.isAsc() ? val : -val;
            }
          });
        }

        ArrayList<Route> res = new ArrayList<Route>(count);
        for (int i=startPos; i<Math.min(routes.size(), startPos + count); i++)
          res.add(routes.get(i));

        return res;
      }

      public int getRowsCount() {
        return routes.size();
      }
    };
  }
}
