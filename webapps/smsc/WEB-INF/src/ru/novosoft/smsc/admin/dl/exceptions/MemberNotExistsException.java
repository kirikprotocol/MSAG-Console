/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 26, 2003
 * Time: 12:10:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl.exceptions;

import ru.novosoft.smsc.admin.AdminException;

public class MemberNotExistsException extends AdminException
{
    public MemberNotExistsException(String member, String dl) {
        super("Member '"+member+"' is not registered in dl '"+dl+"'");
    }
}
