<%@ include file="header.jsp"%>
<h2>Host: <i>smsc</i></h2>
<H3>Services:</H3>
<% boolean isFull = false; %>
<%@ include file="_services_list.jsp"%>
<a href="advanced_add_service.jsp">Add service</a>
<%@ include file="footer.jsp"%>