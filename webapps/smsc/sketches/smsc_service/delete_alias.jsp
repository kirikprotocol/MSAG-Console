<%@ include file="/sketches/header.jsp"%>
<%@ include file="menu.jsp"%>
<h2>Aliases</h2>
<h3>Delete alias:</h3>
Alias "<%=request.getParameter("from")%>&nbsp;-&gt;&nbsp;<%=request.getParameter("to")%>" deleted sucessfully.
<%@ include file="/sketches/footer.jsp"%>