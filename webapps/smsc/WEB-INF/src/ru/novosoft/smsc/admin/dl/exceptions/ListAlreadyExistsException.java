/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 26, 2003
 * Time: 1:02:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class ListAlreadyExistsException extends AdminException
{
    public ListAlreadyExistsException(String dl) {
        super("List '"+dl+"' already exists");
    }
}
