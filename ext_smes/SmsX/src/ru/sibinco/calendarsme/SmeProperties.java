package ru.sibinco.calendarsme;

import ru.sibinco.calendarsme.utils.Utils;
import ru.sibinco.calendarsme.engine.timezones.Timezones;

import java.util.Properties;

/**
 * User: artem
 * Date: 10.11.2006
 */

public class SmeProperties {

  public static final class General {
    // REQUEST PROCESSOR
    // SQL
    public static final String REQUEST_PROCESSOR_INSERT_MESSAGE_SQL;
    public static final String REQUEST_PROCESSOR_TIMEZONES_XML;
    public static final String REQUEST_PROCESSOR_ROUTES_XML;
    public static final int REQUEST_PROCESSOR_RELOAD_TIMEZONES_INTERVAL;

    // CALENDAR ENGINE
    public static final long CALENDAR_ENGINE_WORKING_INTERVAL;
    // SQL
    public static final String CALENDAR_ENGINE_LOAD_LIST_SQL;
    public static final String CALENDAR_ENGINE_REMOVE_MESSAGE_SQL;

    // CALENDAR MESSAGES LIST
    public static final int CALENDAR_MESSAGES_LIST_MAX_SIZE;

    // SECRET ENGINE
    public static final String SECRET_ENGINE_SME_ADDRESS;
    // MESSAGEs
    public static final String SECRET_ENGINE_REGISTER_OK_MESSAGE;
    public static final String SECRET_ENGINE_REGISTER_ERROR_MESSAGE;
    public static final String SECRET_ENGINE_UNREGISTER_OK_MESSAGE;
    public static final String SECRET_ENGINE_UNREGISTER_ERROR_MESSAGE;
    public static final String SECRET_ENGINE_INFORM_MESSAGE;
    public static final String SECRET_ENGINE_NO_MESSAGES;
    public static final String SECRET_ENGINE_DELIVERY_REPORT;
    public static final String SECRET_ENGINE_WRONG_PASSWORD;
    public static final String SECRET_ENGINE_DESTINATION_ABONENT_NOT_REGISTERED;
    public static final String SECRET_ENGINE_ABONENT_NOT_REGISTERED;
    public static final String SECRET_ENGINE_SYSTEM_ERROR;
    public static final String SECRET_ENGINE_ABONENT_ALREADY_REGISTERED;
    public static final String SECRET_ENGINE_PASSWORD_CHANGED;


    static {
      final java.util.Properties config = Utils.loadConfig("sme.properties");

      REQUEST_PROCESSOR_INSERT_MESSAGE_SQL = Utils.loadString(config, "request.processor.insert.message.sql");
      REQUEST_PROCESSOR_TIMEZONES_XML = Utils.loadString(config, "request.processor.timezones.xml");
      REQUEST_PROCESSOR_ROUTES_XML = Utils.loadString(config, "request.processor.routes.xml");
      REQUEST_PROCESSOR_RELOAD_TIMEZONES_INTERVAL = Utils.loadInt(config, "request.processor.reload.timezones.interval");

      CALENDAR_ENGINE_WORKING_INTERVAL = Utils.loadInt(config, "calendar.engine.working.interval");
      CALENDAR_ENGINE_LOAD_LIST_SQL = Utils.loadString(config, "calendar.engine.load.list.sql");
      CALENDAR_ENGINE_REMOVE_MESSAGE_SQL = Utils.loadString(config, "calendar.engine.remove.message.sql");

      CALENDAR_MESSAGES_LIST_MAX_SIZE = Utils.loadInt(config, "calendar.messages.list.max.size");

      SECRET_ENGINE_SME_ADDRESS = Utils.loadString(config, "secret.engine.sme.address");

      SECRET_ENGINE_REGISTER_OK_MESSAGE = Utils.loadString(config, "secret.engine.register.ok.message");
      SECRET_ENGINE_REGISTER_ERROR_MESSAGE = Utils.loadString(config, "secret.engine.register.error.message");
      SECRET_ENGINE_UNREGISTER_OK_MESSAGE = Utils.loadString(config, "secret.engine.unregister.ok.message");
      SECRET_ENGINE_UNREGISTER_ERROR_MESSAGE = Utils.loadString(config, "secret.engine.unregister.error.message");
      SECRET_ENGINE_INFORM_MESSAGE = Utils.loadString(config, "secret.engine.inform.message");
      SECRET_ENGINE_NO_MESSAGES = Utils.loadString(config, "secret.engine.no.messages");
      SECRET_ENGINE_DELIVERY_REPORT = Utils.loadString(config, "secret.engine.delivery.report");
      SECRET_ENGINE_WRONG_PASSWORD = Utils.loadString(config, "secret.engine.wrong.password");
      SECRET_ENGINE_DESTINATION_ABONENT_NOT_REGISTERED = Utils.loadString(config, "secret.engine.destination.abonent.not.registered");
      SECRET_ENGINE_ABONENT_NOT_REGISTERED= Utils.loadString(config, "secret.engine.abonent.not.registered");
      SECRET_ENGINE_SYSTEM_ERROR= Utils.loadString(config, "secret.engine.system.error");
      SECRET_ENGINE_ABONENT_ALREADY_REGISTERED= Utils.loadString(config, "secret.engine.abonent.already.registered");
      SECRET_ENGINE_PASSWORD_CHANGED= Utils.loadString(config, "secret.engine.password.changed");

      config.clear();
    }
  }

  public static final class SecretMessage {
    public static final String INSERT_MESSAGE_SQL;
    public static final String SELECT_MESSAGE_SQL;
    public static final String REMOVE_MESSAGE_SQL;

    static {
      final Properties config = Utils.loadConfig("secretmessage.properties");
      INSERT_MESSAGE_SQL = Utils.loadString(config, "insert.sql");
      REMOVE_MESSAGE_SQL = Utils.loadString(config, "remove.sql");
      SELECT_MESSAGE_SQL = Utils.loadString(config, "select.by.user.sql");
      config.clear();
    }
  }

  public static final class SecretUser {
    public static final String INSERT_SQL;
    public static final String REMOVE_BY_NUMBER_SQL;
    public static final String SELECT_BY_NUMBER_SQL;
    public static final String UPDATE_PASSWORD_SQL;

    static {
      final Properties config = Utils.loadConfig("secretuser.properties");
      INSERT_SQL = Utils.loadString(config, "insert.sql");
      REMOVE_BY_NUMBER_SQL = Utils.loadString(config, "remove.sql");
      SELECT_BY_NUMBER_SQL = Utils.loadString(config, "select.by.number.sql");
      UPDATE_PASSWORD_SQL = Utils.loadString(config, "update.password.sql");
    }
  }

  private SmeProperties() {}

  public static void init() {
    new General();
    new SecretMessage();
    new SecretUser();
    Timezones.init();
  }
}