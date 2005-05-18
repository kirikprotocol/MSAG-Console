package ru.sibinco.scag.backend.protocol.journal;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 18:41:52
 * To change this template use File | Settings | File Templates.
 */
public abstract class SubjectTypes
{
  public static final byte TYPE_UNKNOWN = 0;
  public static final byte TYPE_route = 1;        //
  public static final byte TYPE_alias = 2;        //
  public static final byte TYPE_dl = 3;
  public static final byte TYPE_host = 4;         //
  public static final byte TYPE_locale = 5;
  public static final byte TYPE_profile = 6;
  public static final byte TYPE_schedule = 7;     //
  public static final byte TYPE_service = 8;
  public static final byte TYPE_smsc_config = 9;  //
  public static final byte TYPE_subject = 10;     //
  public static final byte TYPE_user = 11;        //
  public static final byte TYPE_directive = 12;   //
  public static final byte TYPE_logger = 13;
  public static final byte TYPE_msc = 14;
  public static final byte TYPE_securityConstraint = 15;
  public static final byte TYPE_provider = 16;        //
  public static final byte TYPE_category = 17;        //

  public final static String typeToString(byte subjectType)
  {
    switch (subjectType) {
      case TYPE_route:
        return "route";
      case TYPE_alias:
        return "alias";
      case TYPE_dl:
        return "distribution list";
      case TYPE_host:
        return "host";
      case TYPE_locale:
        return "locale";
      case TYPE_profile:
        return "profile";
      case TYPE_schedule:
        return "schedule";
      case TYPE_service:
        return "service";
      case TYPE_smsc_config:
        return "smsc config";
      case TYPE_subject:
        return "subject";
      case TYPE_user:
        return "user";
      case TYPE_directive:
        return "directive";
      case TYPE_logger:
        return "logger";
      case TYPE_msc:
        return "msc";
      case TYPE_securityConstraint:
        return "security constraint";
      case TYPE_provider:
        return "provider";
      case TYPE_category:
        return "category";
      default:
        return "unknown";
    }
  }

  public final static byte stringToType(String subjectType)
  {
    if ("route".equals(subjectType))
      return TYPE_route;
    if ("alias".equals(subjectType))
      return TYPE_alias;
    if ("distribution list".equals(subjectType) || "dl".equals(subjectType))
      return TYPE_dl;
    if ("host".equals(subjectType))
      return TYPE_host;
    if ("locale".equals(subjectType))
      return TYPE_locale;
    if ("profile".equals(subjectType))
      return TYPE_profile;
    if ("schedule".equals(subjectType))
      return TYPE_schedule;
    if ("service".equals(subjectType))
      return TYPE_service;
    if ("smsc config".equals(subjectType))
      return TYPE_smsc_config;
    if ("subject".equals(subjectType))
      return TYPE_subject;
    if ("user".equals(subjectType))
      return TYPE_user;
    if ("directive".equals(subjectType))
      return TYPE_directive;
    if ("logger".equals(subjectType))
      return TYPE_logger;
    if ("msc".equals(subjectType))
      return TYPE_msc;
    if ("security constraint".equals(subjectType))
      return TYPE_securityConstraint;
    if ("provider".equals(subjectType))
      return TYPE_provider;
    if ("category".equals(subjectType))
      return TYPE_category;
    return TYPE_UNKNOWN;
  }
}
