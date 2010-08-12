package ru.sibinco.scag.util;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletException;
import java.io.IOException;
import java.io.ObjectOutputStream;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 16.06.2006
 * Time: 15:50:22
 * To change this template use File | Settings | File Templates.
 */
public class ResourceServlet extends HttpServlet{
    public void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
      System.out.print( "ResourceServlet KEY=" + req.getParameter("key") );
      String line = req.getParameter("key");
      if (line!=null) {
        resp.setCharacterEncoding("UTF-8");
            String value = LocaleMessages.getInstance().getMessage(req.getSession(),line);
            //System.out.println( " VALUE=" + value );
        resp.getWriter().write( value );
      }
      else {
        ObjectOutputStream out = new ObjectOutputStream(resp.getOutputStream());
        out.writeObject(LocaleMessages.getInstance().getLocale(req.getSession()));
        out.close();
      }
    }
}
