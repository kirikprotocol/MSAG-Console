/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 7:28:41 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.Principal;

public class PrincipalAlreadyExistsException extends AdminException
{
    public PrincipalAlreadyExistsException(Principal prc) {
        super("Principal for '"+prc.getAddress()+"' already defined");
    }
}
