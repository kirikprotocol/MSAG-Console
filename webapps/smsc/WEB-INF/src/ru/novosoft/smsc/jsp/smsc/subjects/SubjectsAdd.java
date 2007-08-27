package ru.novosoft.smsc.jsp.smsc.subjects;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.HashSet;
import java.util.Arrays;

public class SubjectsAdd extends SubjectBody {
    protected String mbSave = null;
    protected String mbCancel = null;


    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        if (name == null) {
            name = defSme = "";
//            masks = new String[0];
        }

        if (checkedSources == null)
          checkedSources = new String[0];
        if (srcMasks == null)
          srcMasks = new String[0];

//        if (masks == null)
//            masks = new String[0];

//        masks = Functions.trimStrings(masks);
        srcMasks = Functions.trimStrings(srcMasks);

          checkedSources = Functions.trimStrings(checkedSources);
        checkedSourcesSet = new HashSet(Arrays.asList(checkedSources));

        return result;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (mbCancel != null)
            return RESULT_DONE;
        else if (mbSave != null)
            return save(request);

        return RESULT_OK;
    }

    protected int save(HttpServletRequest request) {
        if (routeSubjectManager.getSubjects().contains(name))
            return error(SMSCErrors.error.subjects.alreadyExists, name);
        else {
//            if (masks == null || masks.length <= 0) {
//                return error(SMSCErrors.error.subjects.masksNotDefined);
//            }
            if ((srcMasks == null || srcMasks.length <= 0) && (checkedSources == null || checkedSources.length <=0)) {
                return error(SMSCErrors.error.subjects.masksNotDefined);
            }
            try {
                routeSubjectManager.getSubjects().add(new Subject(name, srcMasks, smeManager.get(defSme), notes, checkedSources));
                request.getSession().setAttribute("SUBJECT_NAME", name);
                journalAppend(SubjectTypes.TYPE_subject, name, Actions.ACTION_ADD);
                appContext.getStatuses().setSubjectsChanged(true);
                return RESULT_DONE;
            } catch (Throwable e) {
                return error(SMSCErrors.error.subjects.cantAdd, name, e);
            }
        }
    }

//    public List getPossibleSmes() {
//        return smeManager.getSmeNames();
//    }

    /**
     * ************************ properties ********************************
     */
    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

//    public String getName() {
//        return name;
//    }
//
//    public void setName(String name) {
//        this.name = name;
//    }
//
//    public String getDefSme() {
//        return defSme;
//    }
//
//    public void setDefSme(String defSme) {
//        this.defSme = defSme;
//    }
//
//    public String[] getMasks() {
//        return masks;
//    }
//
//    public void setMasks(String[] masks) {
//        this.masks = masks;
//    }
//
//    public String getNotes() {
//        return notes;
//    }
//
//    public void setNotes(String notes) {
//        this.notes = notes;
//    }
}
