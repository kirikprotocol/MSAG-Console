package ru.novosoft.smsc.jsp.smsc.providers;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;

public class ProvidersAdd extends ProvidersEditBean {

    protected int save(final HttpServletRequest request) {
        if (name == null || name.trim().length() == 0)
            return error(SMSCErrors.error.providers.nameNotDefined);

        Provider provider = null;
        try {
            provider = providerManager.getProviderByName(name);
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (provider == null) {
            provider = providerManager.createProvider(name);
            if (provider != null) {
                request.getSession().setAttribute("PROVIDER_NAME", name);
                journalAppend(SubjectTypes.TYPE_provider, name, Actions.ACTION_ADD);
                appContext.getStatuses().setProvidersChanged(true);
                return RESULT_DONE;
            } else
                return error(SMSCErrors.error.providers.couldntCreate);
        } else {
            return error(SMSCErrors.error.providers.providerAlreadyExists, name);
        }
    }
}
