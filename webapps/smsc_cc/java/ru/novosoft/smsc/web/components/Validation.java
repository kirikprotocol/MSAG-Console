package ru.novosoft.smsc.web.components;

/**
 * @author Artem Snopkov
 */
public class Validation {
  public static final Validation PORT = new Validation("port");
  public static final Validation MASK = new Validation("mask");
  public static final Validation ROUTE_MASK = new Validation("routeMask");
  public static final Validation SELECT = new Validation("select");
  public static final Validation PRIORITY = new Validation("priority");
  public static final Validation ROUTE_SERVICE_ID = new Validation("route_serviceId");
  public static final Validation NON_EMPTY = new Validation("nonEmpty");
  public static final Validation EMAIL = new Validation("email");
  public static final Validation POSITIVE = new Validation("positive");
  public static final Validation INT_RANGE = new Validation("int_range");
  public static final Validation ADDRESS = new Validation("address");
  public static final Validation ADDRESS_PREFIX = new Validation("address_prefix");
  public static final Validation ID = new Validation("id");
  public static final Validation UNSIGNED = new Validation("unsigned");
  public static final Validation RESCHEDULE = new Validation("reschedule");
  public static final Validation RELEASE_CAUSE = new Validation("release_cause");
  public static final Validation LANGUAGE = new Validation("language");
  public static final Validation RULE_NAME = new Validation("ruleName");
  public static final Validation HEX = new Validation("hex");

  private final String validation;

  private Validation(String validation) {
    this.validation = validation;
  }

  public String getValidation() {
    return validation;
  }
}
