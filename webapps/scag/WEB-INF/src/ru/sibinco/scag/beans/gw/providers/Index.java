package ru.sibinco.scag.beans.gw.providers;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;


/**
 * Created by igork Date: 30.03.2004 Time: 20:06:32
 */
public class Index extends TabledBeanImpl implements TabledBean {
    
    protected Collection getDataSource() {
        return appContext.getProviderManager().getProviders().values();
    }

    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String providerIdStr = checked[i];
            final Long providerId = Long.decode(providerIdStr);
            toRemove.add(providerId);
        }

        final Map smes = appContext.getSmppManager().getSvcs();
        for (Iterator i = smes.values().iterator(); i.hasNext();) {
            final Svc sme = (Svc) i.next();
            if (toRemove.contains(new Long(sme.getProvider().getId())))
                throw new SCAGJspException(Constants.errors.providers.COULDNT_DELETE_PROVIDER, sme.getProviderName());
        }
        appContext.getProviderManager().getProviders().keySet().removeAll(toRemove);
    }
}