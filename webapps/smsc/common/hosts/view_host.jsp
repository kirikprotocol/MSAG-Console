<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host");%>
<h2>Host: <i><%=host%></i></h2>
<H3>Services:</H3>
<%@ include file="/common/_services_list.jsp"%>
<a href="<%=urlPrefix + servicesPrefix%>/add_adm_service.jsp?host=<%=URLEncoder.encode(host)%>">Add service</a>
<%@ include file="/common/footer.jsp"%>
