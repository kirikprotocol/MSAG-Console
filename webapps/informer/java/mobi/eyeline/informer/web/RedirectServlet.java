package mobi.eyeline.informer.web;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * author: alkhal
 */
public class RedirectServlet extends HttpServlet {
  private static final String URL_PREFIX = "url=";

  @Override
  protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
    String query = req.getQueryString();
    if (query.contains(URL_PREFIX) ) {
      String url = query.substring(query.indexOf(URL_PREFIX)+URL_PREFIX.length());
      url = req.getContextPath() + url;
      System.out.println("url: '"+url+'\'');
      resp.sendRedirect(url);
    }
  }

  @Override
  protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
    doGet(req, resp);
  }

}
