/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 5:48:10 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

public class WSmeErrors
{
  private static String B = "";

  public static class error
  {
    private static final String B = WSmeErrors.B + "error.";

    public static final String failed = B + "failed";
    public static final String unknown = B + "unknown";
    public static final String system = B + "system";

    public static class datasource
    {
      private static final String B = WSmeErrors.error.B + "datasource.";

      public static final String failure = B + "Failure";
    }
    public static class transport
    {
      private static final String B = WSmeErrors.error.B + "transport.";

      public static final String failure = B + "Failure";
    }
    public static class remote
    {
      private static final String B = WSmeErrors.error.B + "remote.";

      public static final String failure = B + "Failure";
    }
    public static class admin
    {
      private static final String B = WSmeErrors.error.B + "admin.";

      public static final String ParseError = B + "ParseError";
    }
  }

  public static class warning
  {
    private static final String B = WSmeErrors.B + "warning.";

    public static class transport
    {
      private static final String B = WSmeErrors.warning.B + "transport.";

      public static final String failure = B + "Failure";
    }
  }

}
