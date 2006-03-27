package ru.novosoft.smsc.jsp.smsc.closedgroups;

import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.Iterator;
import java.util.List;

public class ClosedGroupEdit extends ClosedGroupEditBean {

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        if (name.equals("")) {
            ClosedGroup cg = manager.getClosedGroups().get(Integer.parseInt(id));
            name = cg.getName();
            def = cg.getDef();
            masks = cg.getMasks().getStrings();
        }
        return RESULT_OK;
    }


    protected int save(final HttpServletRequest request) {
        if (name == null || name.trim().length() == 0)
            return error(SMSCErrors.error.closedgroups.nameNotDefined);
//        int idl = Integer.parseInt(id);
        ClosedGroup cg = manager.getClosedGroups().get(Integer.parseInt(id));
        if (cg == null) {
            return error(SMSCErrors.error.closedgroups.groupNotFound, name);
        } else {
            try {
                String newName = "";
                if (!cg.getName().equals(name)) newName = name;
                String newDef = "";
                if (!cg.getDef().equals(def)) newDef = def;
                MaskList add = new MaskList();
                MaskList del = new MaskList();
                MaskList old = cg.getMasks();
                for (int i = 0; i < masks.length; i++) {
                    Mask mask = new Mask(masks[i]);
                    if (!old.contains(mask)) add.add(mask);
                }
                for (Iterator i = old.iterator(); i.hasNext();) {
                    Mask mask = (Mask) i.next();
                    boolean isPresent = false;
                    for (int j = 0; j < masks.length; j++) {
                        if (mask.getMask().equals(masks[j])) {
                            isPresent = true;
                            break;
                        }
                    }
                    if (!isPresent) del.add(mask);
                }
                manager.alter(cg, newName, newDef, add, del);
                request.getSession().setAttribute("CLOSEDGROUP_NAME", name);
                journalAppend(SubjectTypes.TYPE_provider, name, Actions.ACTION_MODIFY);
                appContext.getStatuses().setProvidersChanged(true);
            }
            catch (Exception e) {
                return error(SMSCErrors.error.closedgroups.couldntAlter, e);
            }
            return RESULT_DONE;
        }
    }

}
