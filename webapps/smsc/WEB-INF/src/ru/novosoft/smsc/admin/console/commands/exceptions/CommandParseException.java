/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 17, 2002
 * Time: 11:52:16 AM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.exceptions;

public class CommandParseException extends Exception
{
    public CommandParseException() {
        super();
    }
    public CommandParseException(String message) {
        super(message);
    }
    public CommandParseException(String message, Throwable thr) {
        super(message, thr);
    }
}
