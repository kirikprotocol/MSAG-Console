package ru.sibinco.scag.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;

import java.util.*;


/**
 * Created by igork Date: 22.03.2004 Time: 19:45:12
 */
public class ProviderManager
{
  private final Map providers = Collections.synchronizedMap(new TreeMap());
  private long lastUsedId;
  private final Config idsConfig;
  private static final String SECTION_NAME = "providers";
  private static final String PARAM_NAME_LAST_USED_ID = "last used provider id";


  public ProviderManager(Config idsConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException, NumberFormatException
  {

    lastUsedId = idsConfig.getInt(SECTION_NAME + '.' + PARAM_NAME_LAST_USED_ID);
    this.idsConfig = idsConfig;
    final Collection providerIds = idsConfig.getSectionChildShortParamsNames(SECTION_NAME);
    for (Iterator i = providerIds.iterator(); i.hasNext();) {
      final String providerIdStr = (String) i.next();
      if (!PARAM_NAME_LAST_USED_ID.equalsIgnoreCase(providerIdStr)) {
        final Long providerId = Long.decode(providerIdStr);
        final Provider provider = createProvider(providerId.longValue(), idsConfig.getString(SECTION_NAME + '.' + providerIdStr));
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

  public synchronized void store(final Config idsConfig)
  {
    idsConfig.removeSection(SECTION_NAME);
    idsConfig.setInt(SECTION_NAME + '.' + PARAM_NAME_LAST_USED_ID, lastUsedId);
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      final Provider provider = (Provider) i.next();
      idsConfig.setString(SECTION_NAME + "." + provider.getId(), provider.getName());
    }
  }

  public synchronized void store()
  {
    store(idsConfig);
  }

}
