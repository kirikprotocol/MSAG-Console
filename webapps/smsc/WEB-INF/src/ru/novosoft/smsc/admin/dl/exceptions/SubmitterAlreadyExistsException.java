/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 26, 2003
 * Time: 1:51:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class SubmitterAlreadyExistsException extends AdminException
{
    public SubmitterAlreadyExistsException(String sub, String dl) {
        super("Submitter '"+sub+"' already registered in dl '"+dl+"'");
    }
}
