package ru.sibinco.scag.util;

import java.net.URL;
import java.io.*;
import java.util.HashMap;
import java.util.Locale;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 05.06.2006
 * Time: 17:31:02
 * To change this template use File | Settings | File Templates.
 */
public class RemoteResourceBundle {
  private HashMap properties;
  private URL resource_url;

  public RemoteResourceBundle(URL codebase, String resource_uri) {
    properties = new HashMap();
    try {
        resource_url = new URL(codebase,resource_uri);
        System.out.println( "RRB:codebase='" + codebase.getPath() + "' resource_uri=" + resource_uri );
        System.out.println( "RRB:resource_url=" + resource_url.getPath() + " full:'" + resource_url );
    } catch(Exception e) {
      e.printStackTrace();
    }
  }
  public String getString(String key) {
    System.out.println( "RRB.getString() KEY='" + key + "'" );
    final String codePage = "cp1251";
    String message = "undefined";
    message = (String)properties.get(key);
    System.out.println( "RRB.getString() MSG='" + message + "'" );
    if (message!=null) return message;
    BufferedReader br = null;
    try {
        System.out.println("RRB.getString() STRING for url:'" + resource_url.toString() + " key=" + key + "'");
        URL url = new URL(resource_url.toString() + "key="+key);
        System.out.println("RRB.getString() URL: host=" + url.getHost() + " protocol=" + url.getProtocol() + " path=" + url.getHost() + " port=" + url.getPort());
        InputStream s = url.openStream();
        byte[] mess = new byte[s.available()];
        s.read( mess );
        message = new String(mess, codePage);
        System.out.println("RRB.getString message='" + message + "'\n--------------");
        properties.put(key,message);
    } catch(Exception e) {
      e.printStackTrace();
    } finally {
      if (br!=null)
        try {
          br.close();
        } catch (IOException e) {e.printStackTrace();}
    }
    return message;
  }
  public Locale getLocale() {
    Locale locale=null;
    ObjectInputStream in = null;
    try {
        URL url = new URL(resource_url.toString() + "locale=locale");
        in = new ObjectInputStream(url.openStream());
        locale = (Locale)in.readObject();
    } catch (Exception e) {
        e.printStackTrace();
    } finally {
      if (in!=null) {
        try {
          in.close();
        } catch (IOException e) {e.printStackTrace();}
      }
    }
    return locale;
  }
}
