/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 26, 2003
 * Time: 12:13:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class ListNotExistsException extends AdminException
{
    public ListNotExistsException(String dl) {
        super("List '"+dl+"' not exists");
    }
}
