/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:12:54 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.mscman;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.ArrayList;

public class MscManager
{
    private Smsc smsc = null;

    public void setSmsc(Smsc smsc) {
        this.smsc = smsc;
    }

    private List fakeList() {
        ArrayList lst = new ArrayList();
        lst.add(new MscInfo("32133", false, false, 0));
        lst.add(new MscInfo("12345", true, false, 10));
        lst.add(new MscInfo("54321", true, true, 100));
        lst.add(new MscInfo("32177", false, true, 1000));
        return lst;
    }
    public List list() throws AdminException
    {
        ArrayList mscs = new ArrayList();
        List lst = smsc.mscList();
        for (int i=0; i<lst.size(); i++) {
            Object obj = lst.get(i);
            if (obj != null && obj instanceof String) {
                mscs.add(new MscInfo((String)obj));
            }
        }
        //return fakeList();
        return mscs;
    }
    public void register(String msc) throws AdminException {
        smsc.mscRegistrate(msc);
    }
    public void unregister(String msc) throws AdminException {
        smsc.mscUnregister(msc);
    }
    public void block(String msc) throws AdminException {
        smsc.mscBlock(msc);
    }
    public void clear(String msc) throws AdminException {
        smsc.mscClear(msc);
    }
}
