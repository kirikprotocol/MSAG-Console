<%@ include file="/sketches/header.jsp"%>
<%@ include file="menu.jsp"%>
<h2>Aliases</h2>
<h3>Add alias:</h3>
Alias "<%=request.getParameter("from")%>&nbsp;-&gt;&nbsp;<%=request.getParameter("to")%>" <%=request.getParameter("is_add") != null ? "added" : "edited"%> sucessfully.
<%@ include file="/sketches/footer.jsp"%>