package ru.sibinco.smppgw.beans.gw.providers;

import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.sme.Provider;
import ru.sibinco.smppgw.backend.sme.ProviderManager;
import ru.sibinco.smppgw.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;


/**
 * Created by igork Date: 22.03.2004 Time: 20:04:20
 */
public class Edit extends EditBean
{
  private long id = -1;
  private String name;

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SmppgwJspException
  {
    super.process(request, response);
  }

  protected void load(final String loadId) throws SmppgwJspException
  {
    final Map providers = appContext.getProviderManager().getProviders();
    final Long longLoadId = Long.decode(loadId);
    if (!providers.containsKey(longLoadId))
      throw new SmppgwJspException(Constants.errors.providers.PROVIDER_NOT_FOUND, loadId);

    final Provider info = (Provider) providers.get(longLoadId);
    this.id = info.getId();
    this.name = info.getName();
  }

  protected void save() throws SmppgwJspException
  {
    final ProviderManager providerManager = appContext.getProviderManager();
    if (isAdd()) {
      providerManager.createProvider(name);
    } else {
      providerManager.setProviderName(id, name);
    }
    throw new DoneException();
  }

  public String getId()
  {
    return -1 == id ? null : String.valueOf(id);
  }

  public void setId(final String id)
  {
    this.id = Long.decode(id).longValue();
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    this.name = name;
  }
}
