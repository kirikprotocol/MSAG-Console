package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class ClosedTrapDataSource implements TrapDataSource {


  private TrapDataSource dataSource;

  ClosedTrapDataSource(TrapDataSource dataSource) {
    this.dataSource = dataSource;
  }

  @Override
  public void getTraps(Date from, Date till, SnmpTrapVisitor visitor) throws AdminException {
    final List<_SnmpTrap> traps = new LinkedList<_SnmpTrap>();
    dataSource.getTraps(from, till, new SnmpTrapVisitor() {
      @Override
      public boolean visit(SnmpTrap r) throws AdminException {
        if (r.getSeverity() != 1) { // add new row
          final _SnmpTrap row = new _SnmpTrap(r, false);
          traps.add(row);
        } else { // change rows value

          _SnmpTrap row = null;
          for (_SnmpTrap t : traps) {
            if (r.getAlarmCategory().equals(t.trap.getAlarmCategory()) && r.getAlarmId().equals(t.trap.getAlarmId())) {
              row = t;
              break;
            }
          }

          if (row != null) {
            row.closeFlag = true;
            row = new _SnmpTrap(new SnmpTrap(r), true);
            row.trap.setSeverity(null);
            traps.add(row);
          }
        }
        return true;
      }
    });

    for(_SnmpTrap t : traps) {
      if(t.closeFlag) {
        if(!visitor.visit(t.trap)) {
          return;
        }
      }
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
