package ru.novosoft.smsc.jsp.smsc.closedgroups;

import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class ClosedGroupFilter extends SmscBean {
    private ru.novosoft.smsc.jsp.util.tables.impl.closedgroups.ClosedGroupFilter filter = null;

    private String idFilter = null;
    private String nameFilter = null;
    private String maskFilter = null;
    private String initialized = null;

    private String mbApply = null;
    private String mbClear = null;
    private String mbCancel = null;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        filter = preferences.getClosedGroupFilter();

        if (initialized == null) {
            if (!filter.getIds().isEmpty()) idFilter = ((Integer) filter.getIds().get(0)).toString();
            if (!filter.getNames().isEmpty()) nameFilter = (String) filter.getNames().get(0);
            if (!filter.getMasks().isEmpty()) maskFilter = (String) filter.getMasks().getNames().get(0);
        }
        if (idFilter == null)
            idFilter = "";
        if (nameFilter == null)
            nameFilter = "";
        if (maskFilter == null)
            maskFilter = "";

        return RESULT_OK;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (mbApply != null) {
            String[] temp = new String[1];
            temp[0] = idFilter;
            filter.setIds(temp);
            temp[0] = nameFilter;
            filter.setNames(temp);
            filter.setMasksStrings(maskFilter);
            preferences.setClosedGroupFilter(filter);
            return RESULT_DONE;
        } else if (mbClear != null) {
            idFilter = "";
            nameFilter = "";
            maskFilter = "";
            return RESULT_OK;
        } else if (mbCancel != null)
            return RESULT_DONE;

        return RESULT_OK;
    }

    public String getNameFilter() {
        return nameFilter;
    }

    public void setNameFilter(String nameFilter) {
        this.nameFilter = nameFilter;
    }

    public String getMaskFilter() {
        return maskFilter;
    }

    public void setMaskFilter(String maskFilter) {
        this.maskFilter = maskFilter;
    }

    public String getIdFilter() {
        return idFilter;
    }

    public void setIdFilter(String idFilter) {
        this.idFilter = idFilter;
    }

    public String getMbApply() {
        return mbApply;
    }

    public void setMbApply(String mbApply) {
        this.mbApply = mbApply;
    }

    public String getMbClear() {
        return mbClear;
    }

    public void setMbClear(String mbClear) {
        this.mbClear = mbClear;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

    public String getInitialized() {
        return initialized;
    }

    public void setInitialized(String initialized) {
        this.initialized = initialized;
    }
}
