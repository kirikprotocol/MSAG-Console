/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 3:06:00 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;

import java.util.List;

public class DistributionListAdminFormBean extends IndexBean
{
    private DistributionListAdmin admin = null;

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        return RESULT_OK;
    }

    public int process(SMSCAppContext appContext, List errors)
    {
        if (this.admin == null && this.appContext == null &&
                appContext instanceof SMSCAppContext)
        {
            admin = new DistributionListManager(appContext.getSmsc(),
                            appContext.getConnectionPool());
        }

        int result = super.process(appContext, errors);
        if (result != RESULT_OK)
            return result;

        return result;
    }

}
