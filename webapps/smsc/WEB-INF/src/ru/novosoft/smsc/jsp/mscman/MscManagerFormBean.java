/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:04:15 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.mscman;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.mscman.MscManager;

import java.util.List;
import java.util.ArrayList;

public class MscManagerFormBean extends IndexBean
{
    private MscManager  manager = new MscManager();
    private List        mscs = null;

    private String mbRegister = null;
    private String mbUnregister = null;
    private String mbBlock = null;
    private String mbClear = null;

    private String mscNum = "";

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        return RESULT_OK;
    }

    private void clearBeenProperties() {
        mbRegister = null;  mbUnregister = null;
        mbBlock = null; mbClear = null;
        mscNum = "";
    }
    public int process(SMSCAppContext appContext, List errors)
    {
        if (this.appContext == null && appContext instanceof SMSCAppContext)
        {
            manager.setSmsc(appContext.getSmsc());
        }

        int result = super.process(appContext, errors);
        if (result != RESULT_OK) return result;

        if (mbBlock != null) manager.block(mscNum);
        else if (mbClear != null) manager.clear(mscNum);
        else if (mbUnregister != null) manager.unregister(mscNum);
        else if (mbRegister != null) manager.register(mscNum);

        mscs = manager.list();
        clearBeenProperties();
        return RESULT_OK;
    }

    public List getMscs() {
        return mscs;
    }

    public String getMscNum() {
        return mscNum;
    }
    public void setMscNum(String mscNum) {
        this.mscNum = mscNum;
    }

    public void setMbRegister(String mbRegister) {
        this.mbRegister = mbRegister;
    }
    public void setMbUnregister(String mbUnregister) {
        this.mbUnregister = mbUnregister;
    }
    public void setMbBlock(String mbBlock) {
        this.mbBlock = mbBlock;
    }
    public void setMbClear(String mbClear) {
        this.mbClear = mbClear;
    }
}
