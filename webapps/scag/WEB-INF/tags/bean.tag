<%@
 tag body-content="empty" %><%@
 attribute name="className" required="true"%><%
   String path = request.getServletPath();
   int pos = path.indexOf(".jsp");
   if (pos > 0) {
     int pos2 = path.lastIndexOf('/', pos)+1;
     if (pos2 > 0)
       path = request.getContextPath() + path.substring(0, pos2);
   }

  try {
    ru.sibinco.scag.beans.SCAGBean bean = (ru.sibinco.scag.beans.SCAGBean) Thread.currentThread().getContextClassLoader().loadClass(className).getConstructor(new Class[0]).newInstance(new Object[0]);
    request.setAttribute("bean", bean);
    System.out.println("class = " + bean.getClass().getName() + ", bean = " + bean);
    jspContext.setAttribute("bean", bean); //!pageContext
    %><jsp:setProperty name="bean" property="*"/><%
    bean.process(request, response);
  } catch (ru.sibinco.scag.beans.CancelException e) {
    response.sendRedirect(path);
  } catch (ru.sibinco.scag.beans.DoneException e) {
    response.sendRedirect(path);
  } catch (ru.sibinco.scag.beans.EditException e) {
    response.sendRedirect("edit.jsp?editId=" + java.net.URLEncoder.encode(e.getEditId(), ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()));
  } catch (ru.sibinco.scag.beans.AddException e) {
    response.sendRedirect("add.jsp");
  } catch (ru.sibinco.scag.beans.SCAGJspException e) {
    throw e;
  } catch (Throwable t) {
    out.print("<h1 color=red>bean tag exception: " +(t.getMessage() != null ? '"'+t.getMessage()+'"' : "") + "</h1><pre>");
    t.printStackTrace(new java.io.PrintWriter(out));
    out.print("</pre>");
    final Throwable cause = t.getCause();
    if (cause != null) {
      out.print("<h2 color=red>nested cause: " +(cause.getMessage() != null ? '"'+cause.getMessage()+'"' : "") + "</h2><pre>");
      t.printStackTrace(new java.io.PrintWriter(out));
      out.print("</pre>");
    }
  }
%>