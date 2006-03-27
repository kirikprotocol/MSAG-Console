package ru.novosoft.smsc.jsp.util.tables.impl.closedgroups;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

import java.util.Vector;

public class ClosedGroupQuery extends AbstractQueryImpl {
    public ClosedGroupQuery(int expectedResultsQuantity, ClosedGroupFilter filter, String sortOrder, int startPosition) {
        super(expectedResultsQuantity, filter, sortOrder, startPosition);
    }
}
