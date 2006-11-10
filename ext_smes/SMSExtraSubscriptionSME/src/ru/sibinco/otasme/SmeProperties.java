package ru.sibinco.otasme;

import ru.sibinco.otasme.utils.Utils;

import java.util.Properties;

/**
 * User: artem
 * Date: 10.11.2006
 */

public class SmeProperties {
  public static class SessionsRegistry {
    public static final int MAX_SIZE;
    public static final int CLEAN_INTERVAL;
    public static final int SESSION_IDLE_TIME;

    static {
      final Properties config = Utils.loadConfig("sme.properties");
      MAX_SIZE = Utils.loadInt(config, "sessions.registry.max.size");
      CLEAN_INTERVAL = Utils.loadInt(config, "sessions.registry.clean.interval");
      SESSION_IDLE_TIME = Utils.loadInt(config, "sessions.registry.session.idle.time");
      config.clear();
    }
  }

  public static class Session {
    public static final String ON_ERROR_TEXT;
    public static final String OFF_ERROR_TEXT;
    public static final String INFO_TEXT;
    public static final String UNEXPECTED_MESSAGE_ERROR_TEXT;
    public static final String TIMEOUT_ERROR_TEXT;
    public static final String SMSEXTRA_NUMBER;
    public static final String OTA_NUMBER;
    public static final int MAX_OTA_MESSAGE_REPEATS;

    static {
      final Properties config = Utils.loadConfig("sme.properties");
      ON_ERROR_TEXT = Utils.loadString(config, "session.on.error.text");
      OFF_ERROR_TEXT = Utils.loadString(config, "session.off.error.text");
      INFO_TEXT = Utils.loadString(config, "session.info.text");
      UNEXPECTED_MESSAGE_ERROR_TEXT = Utils.loadString(config, "session.unexpected.message.error.text");
      TIMEOUT_ERROR_TEXT = Utils.loadString(config, "session.timeout.error.text");
      SMSEXTRA_NUMBER = Utils.loadString(config, "session.smsextra.smsc.number");
      OTA_NUMBER = Utils.loadString(config, "sme.engine.ota.number");
      MAX_OTA_MESSAGE_REPEATS = Utils.loadInt(config, "session.max.ota.message.repeats");
      config.clear();
    }
  }

  public static class CommandsRepeater {
    public final static String OTA_NUMBER;
    public final static long RETRY_PERIOD;
    public final static int SIZE;

    static {
      final Properties config = Utils.loadConfig("sme.properties");
      OTA_NUMBER = Utils.loadString(config, "sme.engine.ota.number");
      RETRY_PERIOD = Utils.loadLong(config, "commands.repeater.retry.period");
      SIZE = Utils.loadInt(config, "commands.repeater.size");
      config.clear();
    }
  }

  public static class SmeEngine {
    public static final String NUMBER_NOT_FOUND_ERROR_TEXT;
    public static final String SESSIONS_REGISTRY_OVERFLOW_TEXT;

    static {
      final Properties config = Utils.loadConfig("sme.properties");
      NUMBER_NOT_FOUND_ERROR_TEXT = Utils.loadString(config, "sme.engine.number.not.found.error.text");
      SESSIONS_REGISTRY_OVERFLOW_TEXT = Utils.loadString(config, "sme.engine.sessions.registry.full.text");
      config.clear();
    }
  }

  public static class Templates {
    public static final int RELOAD_INTERVAL;
    public static final String SMSCENTERS_XML;
    public static final String ROUTES_XML;

    static {
      final Properties config = Utils.loadConfig("sme.properties");
      RELOAD_INTERVAL = Utils.loadInt(config, "templates.reload.smscenters.interval");
      SMSCENTERS_XML = Utils.loadString(config, "templates.smscenters.xml");
      ROUTES_XML = Utils.loadString(config, "templates.routes.xml");
      config.clear();
    }
  }

  public static void init() {
    new SessionsRegistry();
    new Session();
    new CommandsRepeater();
    new Templates();
  }
}
