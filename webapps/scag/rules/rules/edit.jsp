<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ page import="ru.sibinco.scag.backend.SCAGAppContext" %>


<c:choose>
    <c:when test="${param.add}">
        <sm:page>
            <jsp:attribute name="title">Create new rule
            </jsp:attribute>
            <jsp:attribute name="menu">
                <sm-pm:menu>
                    <sm-pm:item name="mbNext" value="Next>>>" title="Create rule"/>
                    <sm-pm:item name="mbCancel" value="Cancel" title="Cancel add a new rule" onclick="clickCancel()"/>
                    <sm-pm:space/>
                </sm-pm:menu>
            </jsp:attribute>
            <jsp:body>

            </jsp:body>
        </sm:page>
    </c:when>
    <c:otherwise>
        <%
            SCAGAppContext appContext = (SCAGAppContext) request.getAttribute("appContext");
            String file = request.getParameter("editId");
            long port = 6695;
            if (appContext != null) {
                try {
                    port = appContext.getConfig().getInt("perfmon.appletport");
                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }
        %>
        <applet code="org.gjt.sp.jedit.jEdit.class" width="100%" height="100%" archive="jedit.jar">
            <param name="noplugins" value="-noplugins">
            <param name="homedir" value="applet">
            <param name="username" value="rules">
            <param name="file" value="<%=file%>">
            <param name="servletUrl" value="/scag/applet/myServlet">
        </applet>
    </c:otherwise>
</c:choose>

