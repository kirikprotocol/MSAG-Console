/**
 * Created by igork
 * Date: Dec 2, 2002
 * Time: 4:57:25 PM
 */
package ru.novosoft.smsc.jsp.smsc.providers;

import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

public class ProvidersAdd extends ProvidersEditBean
{
	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (name == null )
		{
			name = "";
			setRoles(new String[0]);

		}
		return RESULT_OK;
	}

	protected int save()
	{
		if (name == null || name.trim().length() == 0)
			return error(SMSCErrors.error.providers.nameNotDefined);

    Provider provider = null;
    try {
      provider = providerManager.getProviderByName(name);
    } catch (Exception e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    if (provider == null)
		{
      provider=providerManager.createProvider(name);
			if (provider!=null)
			{
        journalAppend(SubjectTypes.TYPE_provider, name, Actions.ACTION_ADD);
				appContext.getStatuses().setProvidersChanged(true);
				return RESULT_DONE;
			}
			else
				return error(SMSCErrors.error.unknown);
		}
		else
		{
			return error(SMSCErrors.error.providers.providerAlreadyExists, name);
		}
	}

	public boolean isNew()
	{
		return true;
	}
}
