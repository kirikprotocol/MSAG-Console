/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:57 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class ErrorCounterSet
{
    public int errcode;
    public int counter;

    ErrorCounterSet(int err, int count) {
        errcode = err; counter = count;
    }
}
