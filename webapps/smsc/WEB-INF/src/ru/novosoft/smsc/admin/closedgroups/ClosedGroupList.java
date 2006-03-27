package ru.novosoft.smsc.admin.closedgroups;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.closedgroups.ClosedGroupDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.closedgroups.ClosedGroupQuery;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class ClosedGroupList {
    private Map map = new HashMap();
    private ClosedGroupDataSource dataSource = new ClosedGroupDataSource();

    public ClosedGroupList() {
    }

    public ClosedGroupList(final Element closedGroupElement) throws AdminException {
        NodeList closedGroupsList = closedGroupElement.getElementsByTagName(ClosedGroupManager.SECTION_NAME_group);
        for (int i = 0; i < closedGroupsList.getLength(); i++) {
            Element closedGroupElem = (Element) closedGroupsList.item(i);
            ClosedGroup cg = put(new ClosedGroup(closedGroupElem));
        }
    }

    public synchronized ClosedGroup put(ClosedGroup cg) {
        if (map.containsKey(cg.getName()))
            throw new IllegalArgumentException("Closed group \"" + cg.getName() + "\" already exist");

        dataSource.add(cg);
        map.put(cg.getName(), cg);
        return cg;
    }


    public synchronized ClosedGroup get(String cgName) {
        return (ClosedGroup) map.get(cgName);
    }

    public synchronized ClosedGroup get(int cgId) {
        for (Iterator i = iterator(); i.hasNext();) {
            ClosedGroup cg = (ClosedGroup) i.next();
            if (cg.getId() == cgId) return cg;
        }
        return null;
    }

    public synchronized boolean isEmpty() {
        return map.isEmpty();
    }

    public synchronized Iterator iterator() {
        return map.values().iterator();
    }

    public synchronized ClosedGroup remove(String cgName) {
        ClosedGroup r = (ClosedGroup) map.remove(cgName);
        if (r != null)
            dataSource.remove(r);
        return r;
    }

    public synchronized void rename(String oldCgName, String newCgName) {
        ClosedGroup r = remove(oldCgName);
        r.setName(newCgName);
        put(r);
    }

    public synchronized PrintWriter store(PrintWriter out) {
        for (Iterator i = iterator(); i.hasNext();) {
            ((ClosedGroup) i.next()).store(out);
        }
        return out;
    }

    public synchronized QueryResultSet query(ClosedGroupQuery query) {
        dataSource.clear();
        for (Iterator i = map.values().iterator(); i.hasNext();) {
            ClosedGroup cg = (ClosedGroup) i.next();
            dataSource.add(cg);
        }
        return dataSource.query(query);
    }

    public synchronized boolean contains(String cgName) {
        return map.containsKey(cgName);
    }
}
