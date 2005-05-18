package ru.sibinco.scag.backend;

import ru.sibinco.lib.backend.service.Service;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.service.Type;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.Constants;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;



/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 15:44:02
 * To change this template use File | Settings | File Templates.
 */
public class SCAG  extends Service
{
  private static final String SCAG_COMPONENT_ID = "SCAG";

  private static final String SME_ADD_METHOD_ID = "sme_add";
  private static final String SME_REMOVE_METHOD_ID = "sme_remove";
  private static final String SME_UPDATE_METHOD_ID = "sme_update";
  private static final String SME_STATUS_ID = "sme_status";
  private static final String SCAG_SME_ID = "SCAG";
  private long serviceRefreshTimeStamp = 0;
  private Map smeStatuses = new HashMap();
  public SCAG(ServiceInfo info)
  {
    super(info);
  }
  public SCAG(final String scagHost,final int scagPort,final String smscConfFolderString,  SCAGAppContext smscAppContext) throws SibincoException
  {
  super(new ServiceInfo(SCAF_SME_ID, scagHost, "", "", true, null, ServiceInfo.STATUS_STOPPED), scagPort);
 
  }

  public synchronized void smeAdd(final Sme sme) throws SibincoException
  {
    final Object result = call(SCAG_COMPONENT_ID, SME_ADD_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }

  public synchronized void smeRemove(final String smeId) throws SibincoException
  {
    final Map params = new HashMap();
    params.put("id", smeId);
    final Object result = call(SCAG_COMPONENT_ID, SME_REMOVE_METHOD_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }

  public synchronized void smeUpdate(final Sme sme) throws SibincoException
  {
    final Object result = call(SCAG_COMPONENT_ID, SME_UPDATE_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }
   public synchronized SmeStatus getSmeStatus(final String id) throws SibincoException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear();
      final Object result = call(SCAG_COMPONENT_ID, SME_STATUS_ID, Type.Types[Type.StringListType], new HashMap());
      if (!(result instanceof List))
        throw new SibincoException("Error in response");

      for (Iterator i = ((List) result).iterator(); i.hasNext();) {
        final String s = (String) i.next();
        final SmeStatus smeStatus = new SmeStatus(s);
        smeStatuses.put(smeStatus.getId(), smeStatus);
      }
    }
    return (SmeStatus) smeStatuses.get(id);
  }
   private Map putSmeIntoMap(final Sme sme)
  {
    final Map params = new HashMap();
    params.put("id", sme.getId());
    params.put("priority", new Integer(sme.getPriority()));
    params.put("typeOfNumber", new Integer(sme.getTypeOfNumber()));
    params.put("numberingPlan", new Integer(sme.getNumberingPlan()));
    params.put("interfaceVersion", new Integer(sme.getInterfaceVersion()));
    params.put("systemType", sme.getSystemType());
    params.put("password", sme.getPassword());
    params.put("addrRange", sme.getAddrRange());
    params.put("smeN", new Integer(sme.getSmeN()));
    params.put("wantAlias", new Boolean(sme.isWantAlias()));
    params.put("forceDC", new Boolean(sme.isForceDC()));
    params.put("timeout", new Integer(sme.getTimeout()));
    params.put("receiptSchemeName", sme.getReceiptSchemeName());
    params.put("disabled", new Boolean(sme.isDisabled()));
    params.put("mode", sme.getModeStr());
    params.put("proclimit", new Integer(sme.getProclimit()));
    params.put("schedlimit", new Integer(sme.getSchedlimit()));
    return params;
  }
}