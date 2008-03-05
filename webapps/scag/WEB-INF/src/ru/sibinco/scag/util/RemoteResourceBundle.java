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
    } catch(Exception e) {
      e.printStackTrace();
    }
  }
  public String getString(String key) {
    String message = "undefined";
    message = (String)properties.get(key);
    if (message!=null) return message;
    BufferedReader br = null;
    try {
    URL url = new URL(resource_url.toString() + "key="+key);

        InputStream s =url.openStream();
        byte[] mess = new byte[s.available()];
        s.read( mess );
        message = new String(mess, "cp1251");

//    InputStreamReader is = new InputStreamReader(url.openStream());
//    br = new BufferedReader(new InputStreamReader(url.openStream()));
//    message = br.readLine();
        
    System.out.println("_MESSAGE='" + message );
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
