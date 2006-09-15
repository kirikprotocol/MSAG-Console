package ru.novosoft.smsc.emailsme.backend;

/**
 * User: artem
 * Date: 11.09.2006
 */
public class EmailSmeMessages {
  private final static String B = "emailsme.";

  public static class errors {
    private static final String PREFIX = B + "error.";
    public static final String invalidAddress = PREFIX + "wrong_address";
    public static final String invalidDayLimit = PREFIX + "invalid_day_limit";
    public static final String profileNotFound = PREFIX + "profile_not_found";
    public static final String internalError = PREFIX + "internal_error";
    public static final String cantSaveProfile = PREFIX + "cant_save_profile";
    public static final String cantDeleteProfile = PREFIX + "cant_delete_profile";
  }

  public static class messages {
    private static final String PREFIX = B + "message.";
    public static final String address = PREFIX + "address";
    public static final String searchCriteria = PREFIX + "search_criteria";
    public static final String userID = PREFIX + "user_ID";
    public static final String dayLimit = PREFIX + "day_limit";
    public static final String forwardAddress = PREFIX + "forwardAddress"; 
  }
}
