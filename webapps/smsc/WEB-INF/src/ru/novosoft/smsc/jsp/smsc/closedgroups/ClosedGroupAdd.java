package ru.novosoft.smsc.jsp.smsc.closedgroups;

import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;

public class ClosedGroupAdd extends ClosedGroupEditBean {
    protected int save(final HttpServletRequest request) {
        if (name == null || name.trim().length() == 0)
            return error(SMSCErrors.error.closedgroups.nameNotDefined);

        ClosedGroup cg = null;
        try {
            cg = manager.getClosedGroups().get(name);
            if (cg == null) {
                cg = manager.add(name, def, masks);
                if (cg != null) {
                    request.getSession().setAttribute("CLOSEDGROUP_NAME", name);
                    journalAppend(SubjectTypes.TYPE_closedgroup, name, Actions.ACTION_ADD);
                    appContext.getStatuses().setClosedGroupChanged(true);
                } else
                    return error(SMSCErrors.error.closedgroups.couldntCreate);
            } else {
                return error(SMSCErrors.error.closedgroups.groupAlreadyExists, name);
            }
        } catch (Exception e) {
            return error(SMSCErrors.error.closedgroups.couldntCreate, e);
        }
        return RESULT_DONE;
    }
}
