/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:06:39 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;

import java.util.*;

public class WSmeTransport extends Service
{
  private static final String SME_COMPONENT_ID = Constants.WSME_SME_ID;

  private static final String add_visitor_method_ID = "add_visitor";
  private static final String remove_visitor_method_ID = "remove_visitor";
  private static final String add_lang_method_ID = "add_lang";
  private static final String remove_lang_method_ID = "remove_lang";
  private static final String add_ad_method_ID = "add_ad";
  private static final String remove_ad_method_ID = "remove_ad";


  public WSmeTransport(ServiceInfo info)
          throws AdminException
  {
    super(info);
  }

  synchronized public void addVisitor(String msisdn)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("msisdn", msisdn.trim());
    call(SME_COMPONENT_ID, add_visitor_method_ID, Type.Types[Type.StringType], args);
  }

  synchronized public void removeVisitor(String msisdn)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("msisdn", msisdn.trim());
    call(SME_COMPONENT_ID, remove_visitor_method_ID, Type.Types[Type.StringType], args);
  }

  synchronized public void addLang(String mask, String lang)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("mask", mask.trim());
    args.put("lang", lang.trim());
    call(SME_COMPONENT_ID, add_lang_method_ID, Type.Types[Type.StringType], args);
  }

  synchronized public void removeLang(String mask)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("mask", mask.trim());
    call(SME_COMPONENT_ID, remove_lang_method_ID, Type.Types[Type.StringType], args);
  }

  synchronized public void addAd(int id, String lang, String ad)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("id", new Long(id));
    args.put("lang", lang.trim());
    args.put("ad", ad.trim());
    call(SME_COMPONENT_ID, add_ad_method_ID, Type.Types[Type.StringType], args);
  }

  synchronized public void removeAd(int id, String lang)
          throws AdminException
  {
    Map args = new HashMap();
    args.put("id", new Long(id));
    args.put("lang", lang.trim());
    call(SME_COMPONENT_ID, remove_ad_method_ID, Type.Types[Type.StringType], args);
  }

}
