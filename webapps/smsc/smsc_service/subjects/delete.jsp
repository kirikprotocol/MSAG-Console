<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Subjects</h4>
<%smsc.getSubjects().remove(request.getParameter("Name"));%>
Subject removed sucessfully.
<%@ include file="/common/footer.jsp"%>