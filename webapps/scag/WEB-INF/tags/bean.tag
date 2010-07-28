<%@ tag import="java.io.File"%>
<%@ tag body-content="empty" %>
<%@ attribute name="className" required="true" %>

<%
    String path = request.getServletPath();
    int pos = path.indexOf(".jsp");
    if (pos > 0) {
        int pos2 = path.lastIndexOf('/', pos) + 1;
        if (pos2 > 0)
            path = request.getContextPath() + path.substring(0, pos2);
    }

    try {
        ru.sibinco.scag.beans.SCAGBean bean = (ru.sibinco.scag.beans.SCAGBean) Thread.currentThread().getContextClassLoader().loadClass(className).getConstructor(new Class[0]).newInstance(new Object[0]);
        request.setAttribute("bean", bean);
        //System.out.println("class = " + bean.getClass().getName() + ", bean = " + bean);
        jspContext.setAttribute("bean", bean); //!pageContext
%>
<jsp:setProperty name="bean" property="*"/>

<%
        bean.process(request, response);
    } catch (ru.sibinco.scag.beans.CancelException e) {
        response.sendRedirect(path);
    } catch (ru.sibinco.scag.beans.CancelChildException e) {
        response.sendRedirect(e.getPath());
    } catch (ru.sibinco.scag.beans.DoneException e) {
        response.sendRedirect(path);
    } catch (ru.sibinco.scag.beans.EditException e) {
        response.sendRedirect("edit.jsp?editId=" + java.net.URLEncoder.encode(e.getEditId(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()));
    } catch (ru.sibinco.scag.beans.EditChildException e) {
        response.sendRedirect("edit.jsp?editId=" + java.net.URLEncoder.encode(e.getEditId(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()) + "&parentId=" +
                java.net.URLEncoder.encode(e.getParentId(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()));
    } catch (ru.sibinco.scag.beans. AddException e) {
        response.sendRedirect("add.jsp");
    } catch (ru.sibinco.scag.beans. AddSiteSubjectException e) {
        response.sendRedirect("add.jsp?add=true&transportId=" +
                java.net.URLEncoder.encode(e.getTransportId(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding())+ "&subjectType=" +
                java.net.URLEncoder.encode(e.getSubjectType(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()));
    }catch (ru.sibinco.scag.beans.AddChildException e) {
        response.sendRedirect(e.getAdditDir() + File.separatorChar + "add.jsp?add=true&parentId=" +
                java.net.URLEncoder.encode(e.getParentId(),
                ru.sibinco.lib.backend.util.Functions.getLocaleEncoding()));
    } catch (ru.sibinco.scag.beans.SCAGJspException e) {
            String message = null;
            String messageresult = e.getMessage();
            String messagecause = null;
            String messagecauseresult = null;
            if (e.getCode() != null)
              message = ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,e.getCode().getId());
            if (message == null)
              message = (e.getCode()==null)?"Unknown exception":e.getCode().getId();
            String exc=ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,"error.common.exception");
            String info=ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,"error.common.info");
            messageresult = java.text.MessageFormat.format(messageresult,new Object[]{exc,message,info});

            Throwable cause = null;
            //if (e.getCause() instanceof ru.sibinco.scag.beans.SCAGJspException)
               cause = e.getCause();
            if (cause!=null && cause instanceof ru.sibinco.scag.beans.SCAGJspException) {
               cause = (ru.sibinco.scag.beans.SCAGJspException)cause;
               messagecauseresult = cause.getMessage();
            if (((ru.sibinco.scag.beans.SCAGJspException)cause).getCode()!=null) {
              messagecause = ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,((ru.sibinco.scag.beans.SCAGJspException)cause).getCode().getId());
            }
            if (messagecause == null)
                 messagecause = (((ru.sibinco.scag.beans.SCAGJspException)cause).getCode()==null)?"Unknown exception":((ru.sibinco.scag.beans.SCAGJspException)cause).getCode().getId();
            messagecauseresult = java.text.MessageFormat.format(messagecauseresult,new Object[]{exc,messagecause,info});
            } else if (cause!=null) messagecauseresult = ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,"error.common.exception") +" "+cause.getMessage();
            %>
              <div class=error>
                <div class=header><%=messageresult%></div>
             <% if (messagecauseresult!=null) { %>
                <div class=header><%=ru.sibinco.scag.util.LocaleMessages.getInstance().getMessage(session,"error.common.nested")+" "+messagecauseresult%></div>
              </div>
            <%   }
       // throw e;
    } catch (Throwable t) {
        out.print("<h1 color=red>bean tag exception: " + (t.getMessage() != null ? '"' +
                t.getMessage() + '"' : "") + "</h1><pre>");
        t.printStackTrace(new java.io.PrintWriter(out));
        out.print("</pre>");
        final Throwable cause = t.getCause();
        if (cause != null) {
            out.print("<h2 color=red>nested cause: " + (cause.getMessage() != null ? '"' +
                    cause.getMessage() + '"' : "") + "</h2><pre>");
            t.printStackTrace(new java.io.PrintWriter(out));
            out.print("</pre>");
        }
    }
%>