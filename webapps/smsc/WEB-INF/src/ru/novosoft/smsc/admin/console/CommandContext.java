/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 1:44:55 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class CommandContext
{
    protected Smsc smsc;
    protected String message = null;
    protected int result = 0;

    public CommandContext(Smsc smsc) {
        this.smsc = smsc;
    }
    public Smsc getSmsc() {
        return smsc;
    }

    public String getMessage() {
        return message;
    }
    public void setMessage(String message) {
        this.message = message;
    }

    public int getResult() {
        return result;
    }
    public void setResult(int result) {
        this.result = result;
    }
}
