/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 7:32:38 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class MemberAlreadyExistsException extends AdminException
{
    public MemberAlreadyExistsException(String member, String dl) {
        super("Member '"+member+"' already registered in dl '"+dl+"'");
    }
}
