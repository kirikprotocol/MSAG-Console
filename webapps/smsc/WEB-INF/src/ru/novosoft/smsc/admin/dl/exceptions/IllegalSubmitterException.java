/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 26, 2003
 * Time: 12:16:14 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class IllegalSubmitterException extends AdminException
{
    public IllegalSubmitterException(String sub, String dl) {
        super("Submitter '"+sub+"' is not registered in dl '"+dl+"'");
    }
}
