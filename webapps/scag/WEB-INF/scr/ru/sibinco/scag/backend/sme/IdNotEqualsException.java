package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.SibincoException;


/**
 * Created by igork
 * Date: 24.03.2004
 * Time: 17:22:12
 */
public class IdNotEqualsException extends SibincoException
{
  public IdNotEqualsException(String smeId, String providerId)
  {
    super("Sme ID \"" + smeId + "\" not equals to provider ID \"" + providerId + "\"");
  }
}

