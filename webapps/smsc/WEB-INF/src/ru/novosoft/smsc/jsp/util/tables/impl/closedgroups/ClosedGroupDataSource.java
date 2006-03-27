package ru.novosoft.smsc.jsp.util.tables.impl.closedgroups;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;

public class ClosedGroupDataSource extends AbstractDataSourceImpl {
    private static final String[] columnNames = {"id", "name", "masks"};

    public ClosedGroupDataSource() {
        super(columnNames);
    }

    public void add(ClosedGroup cg) {
        super.add(new ClosedGroupDataItem(cg));
    }

    public void remove(ClosedGroup cg) {
        super.remove(new ClosedGroupDataItem(cg));
    }

    public QueryResultSet query(ClosedGroupQuery query_to_run) {
        return super.query(query_to_run);
    }

}
