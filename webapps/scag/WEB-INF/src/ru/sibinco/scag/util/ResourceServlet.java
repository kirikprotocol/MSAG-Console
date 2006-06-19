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
      //System.out.println("key = "+req.getParameter("key"));
      String line = req.getParameter("key");
      if (line!=null) {
        resp.setCharacterEncoding("Cp1251");
        resp.getWriter().write(LocaleMessages.getInstance().getMessage(req.getSession(),line));
      }
      else {
        ObjectOutputStream out = new ObjectOutputStream(resp.getOutputStream());
        out.writeObject(LocaleMessages.getInstance().getLocale(req.getSession()));
        out.close();
      }
    }
}
