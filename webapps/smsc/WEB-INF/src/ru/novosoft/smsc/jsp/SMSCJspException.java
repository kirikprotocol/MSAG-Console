/*
 * Created by igork
 * Date: 25.10.2002
 * Time: 19:38:12
 */
package ru.novosoft.smsc.jsp;

public class SMSCJspException extends Exception
{
  public static final byte ERROR_CLASS_ERROR = 0;
  public static final byte ERROR_CLASS_WARNING = 1;
  public static final byte ERROR_CLASS_MESSAGE = 2;
  public static final byte ERROR_CLASS_UNKNOWN = 3;

	private String param = null;
  private byte errorClass = ERROR_CLASS_ERROR;

  public SMSCJspException(String message, byte errorClass)
  {
    super(message);
    this.errorClass = errorClass;
  }

  public SMSCJspException(String message, byte errorClass, String param)
  {
    super(message);
    this.param = param;
    this.errorClass = errorClass;
  }

  public SMSCJspException(String message, byte errorClass, Throwable cause)
  {
    super(message, cause);
    this.errorClass = errorClass;
  }

  public SMSCJspException(String message, byte errorClass, String param, Throwable cause)
  {
    super(message, cause);
    this.param = param;
    this.errorClass = errorClass;
  }

	public String getParam()
	{
		return param;
	}

  public byte getErrorClass()
  {
    return errorClass;
  }
}
