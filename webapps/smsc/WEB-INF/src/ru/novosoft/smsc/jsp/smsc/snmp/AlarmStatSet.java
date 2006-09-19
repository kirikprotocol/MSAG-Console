package ru.novosoft.smsc.jsp.smsc.snmp;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class AlarmStatSet {
    private ArrayList rows = new ArrayList();

    public void sort(String sortStr) {
        boolean negative = false;
        if (sortStr.charAt(0) == '-') {
            negative = true;
            sortStr = sortStr.substring(1);
        }
        int sortmode = 0;
        if (sortStr.equals("submit_time")) sortmode = 1;
        else if (sortStr.equals("alarm_id")) sortmode = 2;
        else if (sortStr.equals("alarmcategory")) sortmode = 3;
        else if (sortStr.equals("severity")) sortmode = 4;
        else if (sortStr.equals("text")) sortmode = 5;
        ArrayList sorted = new ArrayList();
        while (rows.size() > 0) {
            int min = 0;
            for (int i = 1; i < rows.size(); i++) {
                switch (sortmode) {
                    case 1:
                        if (negative) {
                            if (((AlarmStatRow) rows.get(i)).getSubmit_time().compareTo(((AlarmStatRow) rows.get(min)).getSubmit_time()) > 0)
                                min = i;
                        } else
                        if (((AlarmStatRow) rows.get(i)).getSubmit_time().compareTo(((AlarmStatRow) rows.get(min)).getSubmit_time()) < 0)
                            min = i;
                        break;
                    case 2:
                        if (negative) {
                            if (((AlarmStatRow) rows.get(i)).getAlarm_id().compareTo(((AlarmStatRow) rows.get(min)).getAlarm_id()) > 0)
                                min = i;
                        } else
                        if (((AlarmStatRow) rows.get(i)).getAlarm_id().compareTo(((AlarmStatRow) rows.get(min)).getAlarm_id()) < 0)
                            min = i;
                        break;
                    case 3:
                        if (negative) {
                            if (((AlarmStatRow) rows.get(i)).getAlarm_category().compareTo(((AlarmStatRow) rows.get(min)).getAlarm_category()) > 0)
                                min = i;
                        } else
                        if (((AlarmStatRow) rows.get(i)).getAlarm_category().compareTo(((AlarmStatRow) rows.get(min)).getAlarm_category()) < 0)
                            min = i;
                        break;
                    case 4:
                        if (negative) {
                            if (((AlarmStatRow) rows.get(i)).getSeverity() > ((AlarmStatRow) rows.get(min)).getSeverity())
                                min = i;
                        } else
                        if (((AlarmStatRow) rows.get(i)).getSeverity() < ((AlarmStatRow) rows.get(min)).getSeverity())
                            min = i;
                        break;
                    case 5:
                        if (negative) {
                            if (((AlarmStatRow) rows.get(i)).getText().compareTo(((AlarmStatRow) rows.get(min)).getText()) > 0)
                                min = i;
                        } else
                        if (((AlarmStatRow) rows.get(i)).getText().compareTo(((AlarmStatRow) rows.get(min)).getText()) < 0)
                            min = i;
                        break;
                }
            }
            sorted.add(rows.remove(min));
        }
        rows = sorted;
    }

    public int getRowsCount() {
        return rows.size();
    }

    public List getRowsList() {
        return rows;
    }

    public void addRow(AlarmStatRow row) {
        rows.add(row);
    }

    public void addAll(Collection smes) {
        rows.addAll(smes);
    }

    public void removeRow(AlarmStatRow row) {
        rows.remove(row);
    }

    public void clean() {
        rows.clear();
    }

    public AlarmStatRow getRow(int index) {
        return ((AlarmStatRow) rows.get(index));
    }
}
