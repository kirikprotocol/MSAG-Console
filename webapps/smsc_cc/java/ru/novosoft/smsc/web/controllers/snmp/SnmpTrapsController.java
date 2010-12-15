package ru.novosoft.smsc.web.controllers.snmp;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.snmp.SnmpFilter;
import ru.novosoft.smsc.admin.snmp.SnmpTrap;
import ru.novosoft.smsc.admin.snmp.SnmpTrapVisitor;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SnmpTrapsController extends SmscController{

  private static final Logger logger = Logger.getLogger(SnmpTrapsController.class);

  private Date dateToView = new Date();

  private SnmpFilter.Type type = SnmpFilter.Type.ALL;

  private boolean init;

  public void query() {
    init = dateToView != null;
    if(!init) {
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.snmp.traps.date.empty");
    }
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public String getType() {
    return type == null ? null : type.toString();
  }

  public void setType(String type) {
    this.type = type == null || type.length() == 0 ? null : SnmpFilter.Type.valueOf(type);
  }

  public Date getDateToView() {
    return dateToView;
  }

  public void setDateToView(Date dateToView) {
    this.dateToView = dateToView;
  }

  public List<SelectItem> getAvailableTypes() {
    List<SelectItem> types = new LinkedList<SelectItem>();
    for(SnmpFilter.Type t : SnmpFilter.Type.values()) {
      String toS = t.toString();
      types.add(new SelectItem(toS, getLocalizedString("smsc.snmp.traps."+toS.toLowerCase())));
    }
    return types;
  }

  public boolean isSortAvailable() {
    return type != SnmpFilter.Type.CLOSED;
  }

  public DataTableModel getTraps() {
    if(!init) {
      return new EmptyDataTableModel();
    }
    Calendar c = Calendar.getInstance();
    c.setTime(dateToView);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    c.add(Calendar.DAY_OF_MONTH, -1);
    Date from = c.getTime();
    c.add(Calendar.DAY_OF_MONTH, 2);
    Date till = c.getTime();

    SnmpFilter filter = new SnmpFilter(type);
    filter.setFrom(from);
    filter.setTill(till);

    final LinkedList<SnmpTrap> traps = new LinkedList<SnmpTrap>();
    try {
      WebContext.getInstance().getSnmpManager().getTraps(filter, new SnmpTrapVisitor() {
        public boolean visit(SnmpTrap r) throws AdminException {
          traps.add(r);
          return true;
        }
      });
    } catch (AdminException e) {
      addError(e);
    }

    final int total = traps.size();

    return new DataTableModel() {
      @Override
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<SnmpTrap> result = new ArrayList<SnmpTrap>(count);

        if (count <= 0) {
          return result;
        }

        if(sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("date")) {
          Collections.sort(traps, new Comparator<SnmpTrap>() {
            public int compare(SnmpTrap o1, SnmpTrap o2) {
              return (o1.getSubmitDate().compareTo(o2.getSubmitDate())) *( sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("alarmId")) {
          Collections.sort(traps, new Comparator<SnmpTrap>() {
            public int compare(SnmpTrap o1, SnmpTrap o2) {
              return (o1.getAlarmId().compareTo(o2.getAlarmId())) *( sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("alarmCategory")) {
          Collections.sort(traps, new Comparator<SnmpTrap>() {
            public int compare(SnmpTrap o1, SnmpTrap o2) {
              return (o1.getAlarmCategory().compareTo(o2.getAlarmCategory())) *( sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("severity")) {
          Collections.sort(traps, new Comparator<SnmpTrap>() {
            public int compare(SnmpTrap o1, SnmpTrap o2) {
              if(o1.getSeverity() == null && o2.getSeverity() != null) {
                return (sortOrder.isAsc() ? -1 : 1);
              }
              if(o1.getSeverity() != null && o2.getSeverity() == null) {
                return (sortOrder.isAsc() ? 1 : -1);
              }
              if(o1.getSeverity() == null && o2.getSeverity() == null) {
                return 0;
              }
              return (o1.getSeverity().compareTo(o2.getSeverity())) *( sortOrder.isAsc() ? 1 : -1);
            }
          });
        }

        for (Iterator<SnmpTrap> i = traps.iterator(); i.hasNext() && count > 0;) {
          SnmpTrap r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }

        return result;
      }

      @Override
      public int getRowsCount() {
        return total;
      }
    };
  }

  private static class EmptyDataTableModel implements DataTableModel{

    @Override
    public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
      return Collections.emptyList();
    }

    @Override
    public int getRowsCount() {
      return 0;
    }
  }


}
