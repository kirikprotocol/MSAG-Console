<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Subjects</h4>
<%routeManager.getSubjects().remove(request.getParameter("name"));%>
Subject removed sucessfully.
<%@ include file="/common/footer.jsp"%>