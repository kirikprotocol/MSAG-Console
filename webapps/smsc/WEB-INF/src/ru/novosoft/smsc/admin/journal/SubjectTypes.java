package ru.novosoft.smsc.admin.journal;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.11.2003
 * Time: 16:38:47
 */
public abstract class SubjectTypes
{
  public static final byte TYPE_UNKNOWN = 0;
  public static final byte TYPE_route = 1;

  public final static String typeToString(byte subjectType)
  {
    switch (subjectType) {
      case TYPE_route:
        return "route";
      default:
        return "unknown";
    }
  }

  public final static byte stringToType(String subjectType)
  {
    if ("route".equals(subjectType))
      return TYPE_route;
    return TYPE_UNKNOWN;
  }
}
