/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * The <code>TabledEditBeanImpl</code> class represents
 * <p><p/>
 * Date: 26.02.2006
 * Time: 17:07:30
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public abstract class TabledEditBeanImpl extends TabledBeanImpl {

    private String editId = null;
    private boolean add = false;
    private String mbSave = null;
    private String mbCancel = null;
    
    public abstract String getId();

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);


        if (getMbCancel() != null)
            throw new CancelException();
        if (getMbSave() != null)
            save();
    }

    protected abstract void save() throws SCAGJspException;

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        this.editId = editId;
    }

    public boolean isAdd() {
        return add;
    }

    public void setAdd(boolean add) {
        this.add = add;
    }

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
}

