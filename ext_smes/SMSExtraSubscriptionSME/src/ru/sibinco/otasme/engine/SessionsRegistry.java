package ru.sibinco.otasme.engine;

import org.apache.log4j.Category;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.utils.Service;

import java.util.*;

/**
 * User: artem
 * Date: 19.10.2006
 */

public class SessionsRegistry {
  private final static Category log = Category.getInstance(SessionsRegistry.class);

  private static final SessionsRegistry instance;

  static {
    instance = new SessionsRegistry(SmeProperties.SessionsRegistry.MAX_SIZE,
                                    SmeProperties.SessionsRegistry.CLEAN_INTERVAL,
                                    SmeProperties.SessionsRegistry.SESSION_IDLE_TIME);
  }

  public static void registerSession(final Session session) throws SessionsRegistryException, SessionsRegistryOverflowException {
    log.info("SESSIONS REGISTRY: Register session with id = " + session.getId());
    instance.registerSessionInternal(session);
  }

  public static void unregisterSession(final Session session) throws SessionsRegistryException {
    log.info("SESSIONS REGISTRY: Unregister session with id = " + session.getId());
    instance.unregisterSessionInternal(session);
  }

  public static Session getSession(final String sessionId) {
    return instance.getSessionInternal(sessionId);
  }

  // IMPLEMENTATION

  private final int maxRegistrySize;
  private final int maxSessionIdleTime;
  private final HashMap registry = new HashMap();

  private SessionsRegistry(int maxRegistrySize, int cleanRegistryInterval, int maxSessionIdleTime) {
    this.maxRegistrySize = maxRegistrySize;
    this.maxSessionIdleTime = maxSessionIdleTime;
    new RegistryCleaner(this, cleanRegistryInterval).startService();
  }

  private void registerSessionInternal(final Session session) throws SessionsRegistryException, SessionsRegistryOverflowException {
    if (session.getId() == null)
      throw new SessionsRegistryException("Can't register session with empty id");

    if (registry.keySet().contains(session.getId()))
      throw new SessionsRegistryException("Session with id=" + session.getId() + " has already been in registry");

    synchronized (registry) {
      if (registry.size() >= maxRegistrySize)
        throw new SessionsRegistryOverflowException();

      registry.put(session.getId(), session);
    }
  }

  private void unregisterSessionInternal(final Session session) throws SessionsRegistryException {
    if (session.getId() == null)
      throw new SessionsRegistryException("Can't unregister session with empty id");

    synchronized (registry) {
      registry.remove(session.getId());
    }
  }

  private Session getSessionInternal(final String sessionId) {
    if (sessionId == null)
      return null;

    synchronized (registry) {
      final Session session = (Session)registry.get(sessionId);
      if (session != null)
        session.setLastRequestDate(new Date());
      return session;
    }
  }

  private void clearRegistry() {
    final Date now = new Date();
    synchronized (registry) {
      final List sessions2remove = new ArrayList();
      for (Iterator iter = registry.values().iterator(); iter.hasNext(); ) {
        final Session session = (Session)iter.next();
        if (now.getTime() - session.getLastRequestDate().getTime() > maxSessionIdleTime) {
          log.info("Remove session with id=" + session.getId() + " by timeout");
          session.doBeforeUnregisterByTimeout();
          sessions2remove.add(session.getId());
        }
      }

      for (Iterator iter = sessions2remove.iterator(); iter.hasNext();) {
        registry.remove(iter.next());
      }
    }
  }

  public static final class SessionsRegistryException extends Exception {
    public SessionsRegistryException(String message) {
      super(message);
    }
  }

  public static final class SessionsRegistryOverflowException extends Exception {}

  private static class RegistryCleaner extends Service {

    private final int cleanInterval;
    private final SessionsRegistry sessionsRegistry;

    RegistryCleaner(final SessionsRegistry sessionsRegistry, final int cleanInterval) {
      super(log);
      this.cleanInterval = cleanInterval;
      this.sessionsRegistry = sessionsRegistry;
    }

    public synchronized void iterativeWork() {

      try {
        wait(cleanInterval);
      } catch (InterruptedException e) {
        log.error("Registry cleaner error: ", e);
      }

      sessionsRegistry.clearRegistry();
    }
  }
}
