package ru.sibinco.scag.util;


import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpServlet;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 18.07.2006
 * Time: 14:02:18
 * To change this template use File | Settings | File Templates.
 */
public class RemoteClassLoader extends HttpServlet {
  private static final String packagePrefix = "/ru/sibinco/scag/";
  private static final String extension = ".class";
  public void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
    try {
    String requestedURI=req.getRequestURI();
    String requestedClassPath = requestedURI.substring(requestedURI.indexOf(packagePrefix));
    String requestedClass = requestedClassPath.substring(1,requestedClassPath.indexOf(extension)).replace('/','.');
    //System.out.println("requestedClassPath = " + requestedClassPath);"/ru/sibinco/scag/beans/rules/RuleState.class"
    //System.out.println("requestedClass = " + requestedClass);"ru.sibinco.scag.beans.rules.RuleState"
    InputStream is = Class.forName(requestedClass).getResourceAsStream(requestedClassPath);
    byte[] buf = new byte[32768];
    int read;
    while ((read=is.read(buf,0,buf.length))!=-1) {
     resp.getOutputStream().write(buf,0,read);
    }
     resp.getOutputStream().flush();
     resp.getOutputStream().close();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
}
