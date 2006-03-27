package ru.novosoft.smsc.jsp.smsc.providers;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public abstract class ProvidersEditBean extends SmscBean {
    protected ProviderManager providerManager = null;
    protected String name = null;
    protected String id = null;
    protected String mbCancel = null;
    protected String mbSave = null;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        providerManager = appContext.getProviderManager();
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

    /**
     * *************************************** properties ***************************************************
     */
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
