/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 17, 2002
 * Time: 11:54:40 AM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.exceptions;

public class CommandProcessException extends Exception
{
    public CommandProcessException() {
        super();
    }
    public CommandProcessException(String message) {
        super(message);
    }
    public CommandProcessException(String message, Throwable thr) {
        super(message, thr);
    }
}

