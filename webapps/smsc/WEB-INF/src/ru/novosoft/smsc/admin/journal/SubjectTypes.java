package ru.novosoft.smsc.admin.journal;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.11.2003
 * Time: 16:38:47
 */
public abstract class SubjectTypes {
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
    public static final byte TYPE_closedgroup = 18;
    public static final byte TYPE_statview = 19;
    public static final byte TYPE_smsview = 20;
    public static final byte TYPE_snmp = 21;
    public static final byte TYPE_maplimits = 22;
    public static final byte TYPE_infosme = 23;
    public static final byte TYPE_regions = 24;
    public static final byte TYPE_network_profiles = 25;


    public final static String typeToString(byte subjectType) {
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
            case TYPE_closedgroup:
                return "closed group";
            case TYPE_statview:
                return "stat view";
            case TYPE_smsview:
                return "sms view";
            case TYPE_snmp:
                return "snmp";
            case TYPE_maplimits:
                return "map limits";
            case TYPE_infosme:
                return "infosme";
            case TYPE_regions:
                return "regions";
            case TYPE_network_profiles:
                return "network profiles";
            default:
                return "unknown";
        }
    }

    public final static byte stringToType(String subjectType) {
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
        if ("closed group".equals(subjectType))
            return TYPE_closedgroup;
        if ("stat view".equals(subjectType))
            return TYPE_statview;
        if ("sms view".equals(subjectType))
            return TYPE_smsview;
        if ("snmp".equals(subjectType))
            return TYPE_snmp;
        if ("map limits".equals(subjectType))
            return TYPE_maplimits;
        if ("infosme".equals(subjectType))
            return TYPE_infosme;
        if ("regions".equals(subjectType))
            return TYPE_regions;
        if ("network profiles".equals(subjectType))
            return TYPE_network_profiles;
        return TYPE_UNKNOWN;
    }
}
