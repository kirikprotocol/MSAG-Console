package ru.novosoft.smsc.jsp.smsc.closedgroups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroupManager;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.closedgroups.ClosedGroupQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class Index extends IndexBean {
    public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
    public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
    public static final int RESULT_DELETE = IndexBean.PRIVATE_RESULT + 2;
    public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 3;

    protected QueryResultSet closedGroups = null;
    protected ClosedGroupManager manager = null;

    protected String editId = null;
    protected String mbAdd = null;
    protected String mbDelete = null;
    protected String mbEdit = null;
    protected String[] checkedClosedGroupsNames = new String[0];
    protected Set checkedClosedGroupsNamesSet = new HashSet();

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;
        pageSize = preferences.getClosedGroupsPageSize();
        if (sort != null)
            preferences.setClosedGroupsSortOrder(sort);
        else
            sort = preferences.getProvidersSortOrder();

        manager = appContext.getClosedGroupManager();
        return RESULT_OK;
    }

    public int process(final HttpServletRequest request) {
        super.process(request);
        closedGroups = new EmptyResultSet();

        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (mbAdd != null)
            return RESULT_ADD;
        else if (mbEdit != null)
            return RESULT_EDIT;
        else if (mbDelete != null) {
            int dresult = deleteClosedGroups();
            if (dresult != RESULT_OK)
                return result;
            else
                return RESULT_DONE;
        }

        logger.debug("Providers.Index - process with sorting [" + preferences.getProvidersSortOrder() + "]");
        ClosedGroupQuery query = new ClosedGroupQuery(pageSize, preferences.getClosedGroupFilter(), preferences.getClosedGroupsSortOrder(), startPosition);
        closedGroups = manager.getClosedGroups().query(query);
        if (request.getSession().getAttribute("CLOSEDGROUP_NAME") != null) {
            closedGroups = getClosedGroupsByName((String) request.getSession().getAttribute("CLOSEDGROUP_NAME"));
            request.getSession().removeAttribute("CLOSEDGROUP_NAME");
        }
        totalSize = closedGroups.getTotalSize();

        checkedClosedGroupsNamesSet.addAll(Arrays.asList(checkedClosedGroupsNames));

        return RESULT_OK;
    }

    private QueryResultSet getClosedGroupsByName(String name) {
        boolean found = false;
        QueryResultSet providers = null;
        while (!found) {
            ClosedGroupQuery query = new ClosedGroupQuery(pageSize, preferences.getClosedGroupFilter(), preferences.getClosedGroupsSortOrder(), startPosition);
            providers = manager.getClosedGroups().query(query);
            for (Iterator i = providers.iterator(); i.hasNext();) {
                DataItem item = (DataItem) i.next();
                String al = (String) item.getValue("name");
                if (al.equals(name)) {
                    found = true;
                }
            }
            if (!found) {
                startPosition += pageSize;
            }
        }
        return providers;
    }

    private int deleteClosedGroups() {
        try {
            for (int i = 0; i < checkedClosedGroupsNames.length; i++) {
                String checkedProviderName = checkedClosedGroupsNames[i];
                manager.removeGroup(checkedProviderName);
                journalAppend(SubjectTypes.TYPE_provider, checkedProviderName, Actions.ACTION_DEL);
            }
        }
        catch (AdminException e) {
            return error(SMSCErrors.error.closedgroups.couldntDelete, e);
        }
        setCheckedClosedGroupsNames(new String[0]);
        appContext.getStatuses().setClosedGroupChanged(true);
        return RESULT_OK;
    }

    public boolean isClosedGroupChecked(String name) {
        return checkedClosedGroupsNamesSet.contains(name);
    }

    public ClosedGroupManager getManager() {
        return manager;
    }

    public QueryResultSet getClosedGroups() {
        return closedGroups;
    }

    public String getMbAdd() {
        return mbAdd;
    }

    public String getMbDelete() {
        return mbDelete;
    }

    public String getMbEdit() {
        return mbEdit;
    }

    public String[] getCheckedClosedGroupsNames() {
        return checkedClosedGroupsNames;
    }

    public void setMbAdd(String mbAdd) {
        this.mbAdd = mbAdd;
    }

    public void setMbDelete(String mbDelete) {
        this.mbDelete = mbDelete;
    }

    public void setMbEdit(String mbEdit) {
        this.mbEdit = mbEdit;
    }

    public void setCheckedClosedGroupsNames(String[] checkedClosedGroupsNames) {
        this.checkedClosedGroupsNames = checkedClosedGroupsNames;
    }

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        this.editId = editId;
    }
}
