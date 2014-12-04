package ru.sibinco.scag.beans;

import ru.sibinco.lib.backend.util.SortByPropertyComparator;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.bean.TabledBean;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;


/**
 * Created by igork
 * Date: 09.03.2004
 * Time: 14:29:00
 */
public abstract class TabledBeanImpl extends SCAGBean implements TabledBean {

    public static final String PAGE_SIZE = "pageSize";
    protected List tabledItems = new ArrayList();
    protected String sort = null;
    protected int pageSize = 1;
    protected int totalSize = 0;
    protected int startPosition = 0;
    protected String[] checked = new String[0];
    protected Set checkedSet = new HashSet();

    protected String mbEdit = null;
    protected String mbAdd = null;
    protected String mbDelete = null;

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        HttpSession session = request.getSession();
        if (session.getAttribute(PAGE_SIZE) == null) {
            session.setAttribute(PAGE_SIZE, new Integer(25));
        } else {
            Integer pSize = (Integer) session.getAttribute(PAGE_SIZE);
            if (pSize.intValue() != pageSize && pageSize != 1) {
                session.setAttribute(PAGE_SIZE, new Integer(pageSize));
            }
        }
        pageSize = Integer.parseInt(String.valueOf(session.getAttribute(PAGE_SIZE)));

        if (mbEdit != null)
            throw new EditException(mbEdit);
        if (mbAdd != null)
            throw new AddException();
        if (mbDelete != null) {
            try {
                delete();
            } finally {
                if (sort == null)
                    sort = getDefaultSort();
                final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort));
                totalSize = results.size();
                if (totalSize > startPosition)
                    tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
                else
                    tabledItems = new LinkedList();

            }
        }
        if (sort == null)
            sort = getDefaultSort();
        final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort));
        totalSize = results.size();
        if (totalSize > startPosition)
            tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
        else
            tabledItems = new LinkedList();


    }

    protected abstract Collection getDataSource();

    protected String getDefaultSort() {
        return "id";
    }

    protected abstract void delete() throws SCAGJspException;

    public final List getTabledItems() {
        return tabledItems;
    }

    public final String getSort() {
        return sort;
    }

    public final void setSort(String sort) {
        this.sort = sort;
    }

    public final int getPageSize() {
        return pageSize;
    }

    public final void setPageSize(int pageSize) {
        this.pageSize = pageSize;
    }

    public final int getStartPosition() {
        return startPosition;
    }

    public final void setStartPosition(int startPosition) {
        this.startPosition = startPosition;
    }

    public final int getTotalSize() {
        return totalSize;
    }

    public final String[] getChecked() {
        return checked;
    }

    public final void setChecked(String[] checked) {
        this.checked = checked;
        this.checkedSet.clear();
        this.checkedSet.addAll(Arrays.asList(checked));
    }

    public final boolean isChecked(String id) {
        return checkedSet.contains(id);
    }

    protected final void setTabledItems(List tabledItems) {
        this.tabledItems = tabledItems;
    }

    protected final void setTotalSize(int totalSize) {
        this.totalSize = totalSize;
    }

    public String getMbEdit() {
        return mbEdit;
    }

    public void setMbEdit(String mbEdit) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbEdit' to '"+mbEdit+"'.");
        this.mbEdit = mbEdit;
    }

    public String getMbAdd() {
        return mbAdd;
    }

    public void setMbAdd(String mbAdd) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbAdd' to '"+mbAdd+"'.");
        this.mbAdd = mbAdd;
    }

    public String getMbDelete() {
        return mbDelete;
    }

    public void setMbDelete(String mbDelete) {
        this.mbDelete = mbDelete;
    }
}
