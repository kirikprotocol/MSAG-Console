package ru.novosoft.smsc.jsp.smsc.providers;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

public class ProvidersEdit extends ProvidersEditBean
{
  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (name == null || name.trim().length() == 0) {
      //  setRoles(new String[0]);
      name = "";
      return error(SMSCErrors.error.providers.nameNotDefined);
    }
    else {
/*        Provider provider = providerManager.getProviderByName(name);
        id=String.valueOf(provider.getId());
        if ((provider == null) && (mbSave == null)) {
          return error(SMSCErrors.error.providers.providerNotFound, name);
        }
*/
    }

    return RESULT_OK;
  }

  protected int save()
  {
    if (name == null || name.trim().length() == 0)
      return error(SMSCErrors.error.providers.nameNotDefined);
    Long idl = Long.getLong(id);
    Provider provider = providerManager.getProvider(idl);
    if (provider == null) { // add new provider
      return error(SMSCErrors.error.providers.providerNotFound, name);
    }
    else {

      provider.setName(name);
      journalAppend(SubjectTypes.TYPE_provider, name, Actions.ACTION_MODIFY);
      appContext.getStatuses().setProvidersChanged(true);
      return RESULT_DONE;
    }
  }

  public boolean isNew()
  {
    return false;
  }

}
