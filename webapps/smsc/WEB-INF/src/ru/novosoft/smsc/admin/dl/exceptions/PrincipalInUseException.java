/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 27, 2003
 * Time: 5:06:22 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class PrincipalInUseException extends AdminException
{
    public PrincipalInUseException(String address) {
        super("Principal for address '"+address+"' is in use");
    }
}
