package ru.novosoft.smsc.jsp.util.tables.impl.closedgroups;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;

public class ClosedGroupDataItem extends AbstractDataItem {
    protected ClosedGroupDataItem(ClosedGroup cg) {
        values.put("id", String.valueOf(cg.getId()));
        values.put("name", cg.getName());
        values.put("def", cg.getDef());
        values.put("masks", cg.getMasks());
    }
}
