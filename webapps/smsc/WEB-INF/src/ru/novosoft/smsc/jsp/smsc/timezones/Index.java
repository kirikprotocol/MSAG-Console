package ru.novosoft.smsc.jsp.smsc.timezones;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;

public class Index extends ru.novosoft.smsc.jsp.smsc.reshedule.Index {
    private TimeZones timezones = null;

    private String mbSave = null;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        try {
            timezones = TimeZones.getInstance(appContext);
            if (timezones == null) {
                throw new Throwable();
            }
        }
        catch (Throwable e) {
            logger.error("couldn't instantiate Reshedules", e);
            return error(SMSCErrors.error.smsc.timezones.couldntGetTimeZones, e);
        }

        return result;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        if (mbSave != null)
            return processApply(request);

        return RESULT_OK;
    }

    public boolean isSubjectPresent(String subj) {
        return timezones.isSubjectPresent(subj);
    }

    public Collection getAllSubjects() {
        Collection usedSubjs = getSubjects();
        Collection allSubjs = routeSubjectManager.getSubjects().getNames();
        for (Iterator i = usedSubjs.iterator(); i.hasNext();) {
            allSubjs.remove(i.next());
        }
        return allSubjs;
    }

    public Collection getMasks() {
        return timezones.getMasks();
    }

    public Collection getSubjects() {
        return timezones.getSubjects();
    }

    public String getMaskTimeZone(String mask) {
        try {
            return timezones.getMaskTimeZone(mask).getID();
        }
        catch (AdminException e) {
            return e.getMessage();
        }
    }

    public String getSubjTimeZone(String subj) {
        try {
            return timezones.getSubjectTimeZone(subj).getID();
        }
        catch (AdminException e) {
            return e.getMessage();
        }
    }

    private int processApply(HttpServletRequest request) {
        int result = RESULT_DONE;

        Enumeration parameterNames = request.getParameterNames();

        String newMaskName = null;
        String newMaskValue = null;
        //String newSubjName = null;
        //String newSubjValue = null;

        Collection oldMasks = timezones.getMasks();
        Collection oldSubjects = timezones.getSubjects();

        while (parameterNames.hasMoreElements()) {
            String s = (String) parameterNames.nextElement();
            if (s.equals("newParamName_" + TimeZones.TIMEZONES_MASKS_PREFIX)) {
                newMaskName = request.getParameter(s);
                continue;
            }
            if (s.equals("newParamValue_" + TimeZones.TIMEZONES_MASKS_PREFIX)) {
                newMaskValue = request.getParameter(s);
                continue;
            }
/*            if (s.equals("newParamName_" + TimeZones.TIMEZONES_SUBJECTS_PREFIX)) {
                newSubjName = request.getParameter(s);
                continue;
            }
            if (s.equals("newParamValue_" + TimeZones.TIMEZONES_SUBJECTS_PREFIX)) {
                newSubjValue = request.getParameter(s);
                continue;
            }*/
            if (s.indexOf('.') <= 0) continue;

            final String parameter = request.getParameter(s);
            try {
                if (s.startsWith(TimeZones.TIMEZONES_MASKS_PREFIX)) {
                    s = s.substring(TimeZones.TIMEZONES_MASKS_PREFIX.length() + 1);
                    oldMasks.remove(s);
                    timezones.putMask(s, parameter);
                    continue;
                }
                if (s.startsWith(TimeZones.TIMEZONES_SUBJECTS_PREFIX)) {
                    s = s.substring(TimeZones.TIMEZONES_SUBJECTS_PREFIX.length() + 1);
                    oldSubjects.remove(s);
                    timezones.putSubject(s, parameter);
                }
            }
            catch (AdminException e) {
                return error(SMSCErrors.error.smsc.timezones.couldntSave, e);
            }
        }

        for (Iterator i = oldMasks.iterator(); i.hasNext();) {
            timezones.removeMask((String)i.next());
        }

        for (Iterator i = oldSubjects.iterator(); i.hasNext();) {
            timezones.removeSubject((String)i.next());
        }

        try {
            if (newMaskName != null && !(newMaskName.equals("")) && newMaskValue != null && !(newMaskValue.equals(""))) {
                timezones.putMask(newMaskName, newMaskValue);
            }
/*            if (newSubjName != null && !(newSubjName.equals("")) && newSubjValue != null && !(newSubjValue.equals(""))) {
                timezones.putSubject(newSubjName, newSubjValue);
            }*/
            timezones.save();
        }
        catch (Exception e) {
            return error(SMSCErrors.error.smsc.timezones.couldntSave, e);
        }
        return result;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String[] getAvailableTimeZones() {
        return timezones.getAvailableIDs();
    }

    public String getDefaultTimeZone() {
        return timezones.getDefaultTimeZone();
    }
}
