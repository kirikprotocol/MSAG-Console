/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:14:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.Service;

public class WSme extends WSmeTransport
{
  public WSme(Service wsmeService) throws AdminException
  {
    super(wsmeService.getInfo());
  }
}
