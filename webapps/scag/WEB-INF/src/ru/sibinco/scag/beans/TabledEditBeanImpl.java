package ru.sibinco.scag.beans;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public abstract class TabledEditBeanImpl extends TabledBeanImpl {

    private String editId = null;
    private boolean add = false;
    private String mbSave = null;
    private String mbCancel = null;
    
    public abstract String getId();

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);

        if (mbCancel != null && !mbCancel.isEmpty()) throw new CancelException();
        if (mbSave != null && !mbSave.isEmpty()) save();
    }

    protected abstract void save() throws SCAGJspException;

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        if (logger.isDebugEnabled()) logger.debug("Set 'editId' to '"+editId+"'.");
        this.editId = editId;
    }

    public boolean isAdd() {
        return add;
    }

    public void setAdd(boolean add) {
        if (logger.isDebugEnabled()) logger.debug("Set 'add' to '"+add+"'.");
        this.add = add;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbSave' to '"+mbSave+"'.");
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbCancel' to '"+mbCancel+"'.");
        this.mbCancel = mbCancel;
    }
}

