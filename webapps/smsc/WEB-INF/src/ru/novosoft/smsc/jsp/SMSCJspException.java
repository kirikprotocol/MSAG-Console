/*
 * Created by igork
 * Date: 25.10.2002
 * Time: 19:38:12
 */
package ru.novosoft.smsc.jsp;

public class SMSCJspException extends Exception
{
	protected String param = null;

	public SMSCJspException(String message)
	{
		super(message);
	}

	public SMSCJspException(String message, String param)
	{
		super(message);
		this.param = param;
	}

	public SMSCJspException(String message, Throwable cause)
	{
		super(message, cause);
	}

	public SMSCJspException(String message, String param, Throwable cause)
	{
		super(message, cause);
		this.param = param;
	}
}
