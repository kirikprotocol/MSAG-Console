/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 5:08:17 PM
 */
package ru.novosoft.smsc.admin;


public class AdminException extends Exception
{
	public AdminException(String s)
	{
		super(s);
	}

  public AdminException(String s, Throwable cause)
	{
		super(s, cause);
	}
}
