package ru.sibinco.scag.beans;


import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;


/**
 * Created by igork
 * Date: 11.03.2004
 * Time: 22:05:34
 */
public class SCAGJspException extends SibincoException
{
  private final String additionalInfo;
  private final Constants code;
  
  public SCAGJspException()
  {
    this(null, null, null);
  }

  public SCAGJspException(Constants code)
  {
    this(code, null, null);
  }

  public SCAGJspException(Constants code, Throwable cause)
  {
    this(code, null, cause);
  }

  public SCAGJspException(Constants code, String additionalInfo)
  {
    this(code, additionalInfo, null);
  }

  public SCAGJspException(Constants code, String additionalInfo, Throwable cause)
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
