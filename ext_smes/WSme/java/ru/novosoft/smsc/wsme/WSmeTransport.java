/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:06:39 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;

import org.apache.log4j.Category;

import java.util.Map;
import java.util.HashMap;

public class WSmeTransport extends Service
{
  private Category logger = Category.getInstance(this.getClass());
  private Component wsme_component = null;

  private Method add_visitor_method = null;
  private Method remove_visitor_method = null;
  private Method add_lang_method = null;
  private Method remove_lang_method = null;
  private Method add_ad_method = null;
  private Method remove_ad_method = null;

  public WSmeTransport(ServiceInfo info)
      throws AdminException
  {
    super(info);
  }

  protected void checkComponents()
  {
    System.out.println("WSmeTransport::checkComponents() "+wsme_component);
    if (wsme_component == null)
    {
      wsme_component = (Component)getInfo().getComponents().get(Constants.WSME_SME_ID);
      Map methods = wsme_component.getMethods();
      add_visitor_method = (Method) methods.get("add_visitor");
      remove_visitor_method = (Method) methods.get("remove_visitor");
      add_lang_method = (Method) methods.get("add_lang");
      remove_lang_method = (Method) methods.get("remove_lang");
      add_ad_method = (Method) methods.get("add_ad");
      remove_ad_method = (Method) methods.get("remove_ad");
    }
  }

  synchronized public void addVisitor(String msisdn)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("msisdn", msisdn);
    refreshComponents();
    call(wsme_component, add_visitor_method, Type.Types[Type.StringType], args);
  }
  synchronized public void removeVisitor(String msisdn)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("msisdn", msisdn);
    refreshComponents();
    call(wsme_component, remove_visitor_method, Type.Types[Type.StringType], args);
  }

  synchronized public void addLang(String mask, String lang)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("mask", mask);
    args.put("lang", lang);
    refreshComponents();
    call(wsme_component, add_lang_method, Type.Types[Type.StringType], args);
  }
  synchronized public void removeLang(String mask)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("mask", mask);
    refreshComponents();
    call(wsme_component, remove_lang_method, Type.Types[Type.StringType], args);
  }

  synchronized public void addAd(int id, String lang, String ad)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("id", new Long(id));
    args.put("lang", lang);
    args.put("ad", ad);
    refreshComponents();
    call(wsme_component, add_ad_method, Type.Types[Type.StringType], args);
  }
  synchronized public void removeAd(int id, String lang)
      throws AdminException
  {
    Map args = new HashMap();
    args.put("id", new Long(id));
    args.put("lang", lang);
    refreshComponents();
    call(wsme_component, remove_ad_method, Type.Types[Type.StringType], args);
  }

}
