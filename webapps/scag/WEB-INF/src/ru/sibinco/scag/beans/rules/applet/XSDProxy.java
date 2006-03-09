package ru.sibinco.scag.beans.rules.applet;

import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.*;
import javax.servlet.ServletException;
import java.io.*;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 09.03.2006
 * Time: 15:13:33
 * To change this template use File | Settings | File Templates.
 */
public class XSDProxy extends HttpServlet{
  public void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
    PrintWriter out = resp.getWriter();
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute("appContext");
    //System.out.println("req.getRequestURI() = " + req.getRequestURI());
    LinkedList xsd = appContext.getRuleManager().getSchema(req.getRequestURI());
    for (Iterator i = xsd.iterator();i.hasNext();) {
    out.println(i.next());
    }
    out.flush(); out.close();
  }
  public void doPut(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
     doGet(req,resp);
  }
}
