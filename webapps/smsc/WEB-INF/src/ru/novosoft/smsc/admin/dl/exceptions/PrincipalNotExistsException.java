/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 27, 2003
 * Time: 5:04:44 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class PrincipalNotExistsException extends AdminException
{
    public PrincipalNotExistsException(String address) {
        super("Principal for address '"+address+"' is not registered");
    }
}
