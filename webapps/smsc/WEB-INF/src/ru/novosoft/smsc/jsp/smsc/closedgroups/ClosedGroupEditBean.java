package ru.novosoft.smsc.jsp.smsc.closedgroups;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroupManager;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public abstract class ClosedGroupEditBean extends SmscBean {
    protected ClosedGroupManager manager = null;

    protected String name = "";
    protected String id = "";
    protected String def = "";
    protected String[] masks = new String[0];

    protected String mbCancel = null;
    protected String mbSave = null;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        manager = appContext.getClosedGroupManager();
        masks = Functions.trimStrings(masks);
        return RESULT_OK;
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

    protected abstract int save(final HttpServletRequest request);

    public ClosedGroupManager getManager() {
        return manager;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getDef() {
        return def;
    }

    public void setDef(String def) {
        this.def = def;
    }

    public String[] getMasks() {
        return masks;
    }

    public void setMasks(String[] masks) {
        this.masks = masks;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }
}
