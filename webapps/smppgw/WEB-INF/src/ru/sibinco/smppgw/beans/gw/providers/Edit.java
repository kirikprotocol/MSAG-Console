package ru.sibinco.smppgw.beans.gw.providers;

import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.sme.Provider;
import ru.sibinco.smppgw.backend.sme.ProviderManager;
import ru.sibinco.smppgw.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;


/**
 * Created by igork
 * Date: 22.03.2004
 * Time: 20:04:20
 */
public class Edit extends EditBean
{
  private long id = -1;
  private String name;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SmppgwJspException
  {
    super.process(request, response);
  }

  protected void load(String loadId) throws SmppgwJspException
  {
    final Map providers = appContext.getProviderManager().getProviders();
    final Long longLoadId = Long.decode(loadId);
    if (!providers.containsKey(longLoadId))
      throw new SmppgwJspException(Constants.errors.providers.PROVIDER_NOT_FOUND, loadId);

    Provider info = (Provider) providers.get(longLoadId);
    this.id = info.getId();
    this.name = info.getName();
  }

  protected void save() throws SmppgwJspException
  {
    final ProviderManager providerManager = appContext.getProviderManager();
    if (isAdd()) {
      final Provider provider = new Provider(providerManager.createId(), name);
      providerManager.getProviders().put(new Long(provider.getId()), provider);
    } else {
      Provider provider = (Provider) providerManager.getProviders().get(new Long(id));
      provider.setName(name);
    }
    throw new DoneException();
  }

  public String getId()
  {
    return id == -1 ? null : String.valueOf(id);
  }

  public void setId(String id)
  {
    this.id = Long.decode(id).longValue();
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }
}
