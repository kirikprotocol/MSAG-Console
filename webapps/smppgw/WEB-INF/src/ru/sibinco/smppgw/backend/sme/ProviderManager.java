package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;

import java.util.*;


/**
 * Created by igork Date: 22.03.2004 Time: 19:45:12
 */
public class ProviderManager
{
  private final Map providers = Collections.synchronizedMap(new TreeMap());
  private long lastUsedId;
  private static final String SECTION_NAME_providers = "providers";
  private static final String PARAM_NAME_last_used_id = "last used provider id";

  public ProviderManager(final Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException, NumberFormatException
  {
    lastUsedId = gwConfig.getInt(SECTION_NAME_providers + '.' + PARAM_NAME_last_used_id);
    final Collection providerIds = gwConfig.getSectionChildShortParamsNames(SECTION_NAME_providers);
    for (Iterator i = providerIds.iterator(); i.hasNext();) {
      final String providerIdStr = (String) i.next();
      if (!PARAM_NAME_last_used_id.equalsIgnoreCase(providerIdStr)) {
        final Long providerId = Long.decode(providerIdStr);
        final Provider provider = createProvider(providerId.longValue(), gwConfig.getString(SECTION_NAME_providers + '.' + providerIdStr));
        providers.put(providerId, provider);
      }
    }
  }

  public synchronized Map getProviders()
  {
    return providers;
  }

  private synchronized Provider createProvider(final long id, final String name)
  {
    final Provider provider = new Provider(id, name);
    providers.put(new Long(provider.getId()), provider);
    return provider;
  }

  public synchronized Provider createProvider(final String name)
  {
    return createProvider(++lastUsedId, name);
  }

  public synchronized void setProviderName(final long id, final String name) throws NullPointerException
  {
    final Provider provider = (Provider) providers.get(new Long(id));
    if (null == provider)
      throw new NullPointerException("Provider \"" + id + "\" not found.");
    provider.setName(name);
  }

  public synchronized void store(final Config gwConfig)
  {
    gwConfig.removeSection(SECTION_NAME_providers);
    gwConfig.setInt(SECTION_NAME_providers + '.' + PARAM_NAME_last_used_id, lastUsedId);
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      final Provider provider = (Provider) i.next();
      gwConfig.setString(SECTION_NAME_providers + "." + provider.getId(), provider.getName());
    }
  }
}
