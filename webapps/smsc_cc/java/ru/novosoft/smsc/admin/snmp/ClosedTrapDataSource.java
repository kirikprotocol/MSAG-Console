package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class ClosedTrapDataSource implements TrapDataSource {


  private TrapDataSource dataSource;

  public ClosedTrapDataSource(TrapDataSource dataSource) {
    this.dataSource = dataSource;
  }

  @Override
  public void getTraps(Date from, Date till, SnmpTrapVisitor visitor) throws AdminException {
    final List<_SnmpTrap> map = new LinkedList<_SnmpTrap>();
    dataSource.getTraps(from, till, new SnmpTrapVisitor() {
      @Override
      public boolean visit(SnmpTrap r) throws AdminException {
        if (r.getSeverity() != 1) { // add new row
          final _SnmpTrap row = new _SnmpTrap(r, false);
          map.add(row);
        } else { // change rows value

          _SnmpTrap row = null;

          for (_SnmpTrap t : map) {
            if (r.getAlarmCategory().equals(t.trap.getAlarmCategory()) && r.getAlarmId().equals(t.trap.getAlarmId())) {
              row = t;
              break;
            }
          }

          if (row != null) {
            row.closeFlag = true;
            row = new _SnmpTrap(new SnmpTrap(r), true);
            row.trap.setSeverity(null);
            map.add(row);
          }
        }
        return true;
      }
    });

    for(_SnmpTrap t : map) {
      if(t.closeFlag) {
        if(!visitor.visit(t.trap)) {
          return;
        }
      }
    }

  }

  private static class Key {

    private String alarmId;

    private String alarmcategory;

    private Key(String alarmId, String alarmcategory) {
      this.alarmId = alarmId;
      this.alarmcategory = alarmcategory;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      Key key = (Key) o;

      if (alarmId != null ? !alarmId.equals(key.alarmId) : key.alarmId != null) return false;
      if (alarmcategory != null ? !alarmcategory.equals(key.alarmcategory) : key.alarmcategory != null) return false;

      return true;
    }

    @Override
    public int hashCode() {
      int result = alarmId != null ? alarmId.hashCode() : 0;
      result = 31 * result + (alarmcategory != null ? alarmcategory.hashCode() : 0);
      return result;
    }
  }

  private static class _SnmpTrap {

    private SnmpTrap trap;

    private boolean closeFlag;

    private _SnmpTrap(SnmpTrap trap, boolean closeFlag) {
      this.trap = trap;
      this.closeFlag = closeFlag;
    }
  }

}
