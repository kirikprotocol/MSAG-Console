package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;

import java.util.*;


/**
 * Created by igork
 * Date: 22.03.2004
 * Time: 19:45:12
 */
public class ProviderManager
{
  final private Map providers = Collections.synchronizedMap(new HashMap());
  private long lastUsedId;

  public ProviderManager(Config gwConfig)
  {
    lastUsedId = 0;
  }

  public Map getProviders()
  {
    return providers;
  }

  public synchronized long createId()
  {
    return ++lastUsedId;
  }
}
