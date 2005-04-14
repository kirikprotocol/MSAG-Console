package ru.sibinco.smppgw.beans;

import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 12.04.2005
 * Time: 19:05:09
 * To change this template use File | Settings | File Templates.
 */
public class MessageException   extends SibincoException
{
  private final String additionalInfo;
  private final String code;
  public static final byte ERROR_CLASS_ERROR = 0;
  public static final byte ERROR_CLASS_WARNING = 1;
  public static final byte ERROR_CLASS_MESSAGE = 2;
  public static final byte ERROR_CLASS_UNKNOWN = 3;

   public MessageException()
  {
    this(null, null, null);
  }

  public MessageException(String code)
  {
    this(code, null, null);
  }

  public MessageException(String code, Throwable cause)
  {
    this(code, null, cause);
  }

  public MessageException(String code, String additionalInfo)
  {
    this(code, additionalInfo, null);
  }
  public MessageException(String code, String additionalInfo, Throwable cause)
   {
     super("Exception: " + (code != null ? code : "internal") + (additionalInfo != null && additionalInfo.length() > 0 ? ", info:\"" + additionalInfo + '"' : ""), cause);
     this.code = code;
     this.additionalInfo = additionalInfo;
   }

  public String getAdditionalInfo()
  {
    return additionalInfo;
  }

  public String getCode()
  {
    return code;
  }
}
