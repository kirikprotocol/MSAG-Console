/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 17, 2002
 * Time: 12:01:47 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.exceptions;

public class CommandRegisterException extends Exception
{
    public CommandRegisterException() {
        super();
    }
    public CommandRegisterException(String message) {
        super(message);
    }
    public CommandRegisterException(String message, Throwable thr) {
        super(message, thr);
    }
}

