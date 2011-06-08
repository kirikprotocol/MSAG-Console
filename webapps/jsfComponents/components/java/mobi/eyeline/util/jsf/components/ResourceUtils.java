package mobi.eyeline.util.jsf.components;

import javax.faces.context.ExternalContext;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpSession;
import java.io.*;
import java.net.URL;
import java.net.URLConnection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class ResourceUtils {

  public static final String COMPONENTS_PATH = "/eyelineComponents";

  private static final String PACKAGE;

  static {
    Package p = ResourceUtils.class.getPackage();
    PACKAGE = p == null ? "" : p.getName().replace(".","/");
  }

  /**
   * Возвращает ссылку на ресурс
   * @param resource название ресурса (к примеру, "js/ajax.js")
   * @return ссылка на ресурс
   */
  public static String getResourceUrl(String resource) {
    if(resource.startsWith("/")) {
      resource = resource.substring(1);
    }
    String ctxPath = "faces";
    StringBuilder res =  new StringBuilder(20+resource.length()+ctxPath.length()).append(ctxPath).append(COMPONENTS_PATH);
    if(!resource.startsWith("/")) {
      res.append('/');
    }
    res.append(resource);
    return res.toString();
  }


  /**
   * Записывает содержимое ресурса в респонс
   * @param resource ссылка на ресурс
   * @param externalContext externalContext
   * @throws IOException ошибка при записи
   */
  public static void writeResource(String resource, ExternalContext externalContext) throws IOException{
    int i = resource.indexOf(COMPONENTS_PATH);
    if(i == -1) {
      throw new IllegalArgumentException("Illegal resource urL: "+resource);
    }
    String relative = resource.substring(COMPONENTS_PATH.length()+i);
    URL u =  getResourceUrl(externalContext, PACKAGE + relative);
    if(u != null) {
      copy(u, ((ServletResponse) externalContext.getResponse()).getOutputStream());
    }
  }



  private static void copy(URL url, OutputStream os) throws IOException {
    URLConnection conn = url.openConnection();
    copy(conn.getInputStream(), os);
  }

  private static void copy(InputStream _is, OutputStream _os) throws IOException {
    BufferedInputStream is = null;
    BufferedOutputStream os = null;
    try{
      is = new BufferedInputStream(_is);
      os = new BufferedOutputStream(_os);
      byte[] buffer = new byte[2048];
      int len = 0;
      while((len = is.read(buffer)) > 0) {
        os.write(buffer, 0, len);
      }
    }finally {
      try{
        if(is != null) {
          is.close();
        }
      }catch (IOException ignored){}
      try{
        if(os != null) {
          os.close();
        }
      }catch (IOException ignored){}
    }

  }


  private static final int	CACHED_URLS		= 3000;
  private static final String	CACHE_KEY		= "EYELINE_COMPONENTS_URL_CACHE";

  private static URL getResourceUrl(ExternalContext context, String resourcePath) {
    Map urlCache = getResourceURLCache(context);
    URL url = null;

    url = (URL) urlCache.get(resourcePath);
    if(url != null)
      return url;

    ClassLoader loader = Thread.currentThread().getContextClassLoader();
    url = loader.getResource(resourcePath);
    if (url == null) {
      loader = ResourceUtils.class.getClassLoader();
      url = loader.getResource(resourcePath);
    }
    urlCache.put(resourcePath, url);
    return url;
  }


  /* entry cache per session */
  private static Map getResourceURLCache(ExternalContext context) {
    HttpSession session = (HttpSession)(context.getSession(false));
    // session
    Map cache = (Map) session.getAttribute(CACHE_KEY);
    if (cache == null) {
      cache = Collections.synchronizedMap(new HashMap(CACHED_URLS));
      session.setAttribute(CACHE_KEY, cache);
    }
    if (cache.size() >= CACHED_URLS) {
      cache.clear();
    }
    return cache;
  }










}
