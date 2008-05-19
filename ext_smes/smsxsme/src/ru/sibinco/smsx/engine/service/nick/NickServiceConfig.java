package ru.sibinco.smsx.engine.service.nick;

import com.eyeline.utils.config.properties.PropertiesConfig;

import java.io.File;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

class NickServiceConfig {
  private final String serviceAddress;
  private final String profilerAddress;

  public NickServiceConfig(String configDir) {
    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "services/nick/service.properties"));

      serviceAddress = config.getString("service.address");
      profilerAddress = config.getString("profiler.address");

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public String getServiceAddress() {
    return serviceAddress;
  }

  public String getProfilerAddress() {
    return profilerAddress;
  }
}
