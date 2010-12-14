package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class OpenedTrapDataSource implements TrapDataSource {

  private TrapDataSource dataSource;

  public OpenedTrapDataSource(TrapDataSource dataSource) {
    this.dataSource = dataSource;
  }

  @Override
  public void getTraps(Date from, Date till, SnmpTrapVisitor visitor) throws AdminException {
    final Map<Key, SnmpTrap> map = new HashMap<Key, SnmpTrap>(1000);
    dataSource.getTraps(from, till, new SnmpTrapVisitor() {
      @Override
      public boolean visit(SnmpTrap r) throws AdminException {
        Key k = new Key(r.getAlarmId(), r.getAlarmCategory());
        SnmpTrap row = map.get(k);

        if (r.getSeverity() != 1) { // add new row

          if (row == null) {
            row = new SnmpTrap();
            map.put(k, row);
          } else if (row.getSubmitDate().after(r.getSubmitDate())) {
            return true;
          }

          row.setText(r.getText());
          row.setAlarmCategory(r.getAlarmCategory());
          row.setAlarmId(r.getAlarmId());
          row.setSeverity(r.getSeverity());
          row.setSubmitDate(r.getSubmitDate());

        } else { // remove row
          if (row != null) {
            map.remove(k);
          }
        }
        return true;
      }
    });

    for(SnmpTrap t : map.values()) {
      if(!visitor.visit(t)) {
        return;
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
}
