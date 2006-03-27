package ru.novosoft.smsc.jsp.util.tables.impl.closedgroups;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

public class ClosedGroupFilter implements Filter {
    private List ids = null;
    private List names = null;
    private MaskList masks = null;

    public ClosedGroupFilter() {
        ids = new Vector();
        names = new Vector();
        masks = new MaskList();
    }

    public boolean isEmpty() {
        return ids.isEmpty() && names.isEmpty() && masks.isEmpty();
    }

    protected boolean isNamesAllowed(String name) {
        if (names.isEmpty()) return true;

        if (names.contains(name) || names.contains(name.toLowerCase()) || names.contains(name.toUpperCase()))
            return true;

        for (Iterator j = names.iterator(); j.hasNext();) {
            String subject = (String) j.next();
            if (name.toLowerCase().indexOf(subject.toLowerCase()) != -1)
                return true;
            if (name.toLowerCase().matches(subject.toLowerCase()))
                return true;
        }
        return false;
    }

    protected boolean isIdAllowed(Integer id) {
        if (ids.isEmpty()) return true;

        if (ids.contains(id))
            return true;

        return false;
    }

    protected boolean isMasksAllowed(MaskList itemMasks) {
        if (masks.isEmpty()) return true;

        for (Iterator i = masks.iterator(); i.hasNext();) {
            Mask mask = (Mask) i.next();
            if (itemMasks.contains(mask)) return true;
        }
        return false;
    }

    public boolean isItemAllowed(DataItem item) {
        if (isEmpty())
            return true;
        String itemName = (String) item.getValue("name");
        Integer itemId = Integer.decode((String) item.getValue("id"));
        MaskList itemMasks = (MaskList) item.getValue("masks");

        return isNamesAllowed(itemName) && isIdAllowed(itemId) && isMasksAllowed(itemMasks);
    }

    public void setNames(String[] names) {
        this.names = Arrays.asList(names);
    }

    public void setIds(String[] ids) {
        this.ids.clear();
        for (int i = 0; i < ids.length; i++) {
            try {
                this.ids.add(Integer.valueOf(ids[i]));
            }
            catch(Exception e) {}
        }
    }

    public void setMasksStrings(String masks) {
        this.masks = new MaskList(masks);
    }

    public List getIds() {
        return ids;
    }

    public List getNames() {
        return names;
    }

    public MaskList getMasks() {
        return masks;
    }
}
