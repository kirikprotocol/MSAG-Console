package ru.sibinco.smsx.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public class ShutdownIndicator {

  private boolean shutdown;

  public void shutdown() {
    shutdown = true;
  }

  public boolean isShutdown() {
    return shutdown;
  }
}
