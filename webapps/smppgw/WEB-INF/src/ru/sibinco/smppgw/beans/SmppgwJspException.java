package ru.sibinco.smppgw.beans;


import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.Constants;


/**
 * Created by igork
 * Date: 11.03.2004
 * Time: 22:05:34
 */
public class SmppgwJspException extends SibincoException
{
  private final String additionalInfo;
  private final Constants code;

  public SmppgwJspException()
  {
    this(null, null, null);
  }

  public SmppgwJspException(Constants code)
  {
    this(code, null, null);
  }

  public SmppgwJspException(Constants code, Throwable cause)
  {
    this(code, null, cause);
  }

  public SmppgwJspException(Constants code, String additionalInfo)
  {
    this(code, additionalInfo, null);
  }

  public SmppgwJspException(Constants code, String additionalInfo, Throwable cause)
  {
    super("Exception: " + (code != null ? code.getId() : "internal") + (additionalInfo != null && additionalInfo.length() > 0 ? ", info:\"" + additionalInfo + '"' : ""), cause);
    this.code = code;
    this.additionalInfo = additionalInfo;
  }

  public Constants getCode()
  {
    return code;
  }

  public String getAdditionalInfo()
  {
    return additionalInfo;
  }
}
