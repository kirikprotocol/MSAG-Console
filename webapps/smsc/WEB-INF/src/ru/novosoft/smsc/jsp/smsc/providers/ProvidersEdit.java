package ru.novosoft.smsc.jsp.smsc.providers;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class ProvidersEdit extends ProvidersEditBean {

    protected int save(final HttpServletRequest request) {
        if (name == null || name.trim().length() == 0)
            return error(SMSCErrors.error.providers.nameNotDefined);
        Long idl = Long.decode(id);
        Provider provider = providerManager.getProvider(idl);
        if (provider == null) { // add new provider
            return error(SMSCErrors.error.providers.providerNotFound, name);
        } else {
            provider.setName(name);
            request.getSession().setAttribute("PROVIDER_NAME", name);
            journalAppend(SubjectTypes.TYPE_provider, name, Actions.ACTION_MODIFY);
            appContext.getStatuses().setProvidersChanged(true);
            return RESULT_DONE;
        }
    }
}
