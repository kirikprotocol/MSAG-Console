<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
TITLE = getLocString("help.title");
MENU0_SELECTION = "MENU0_NONE";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<a href="SMSC Administration Guide.doc"><%=getLocString("help.downloadadmin")%></a><br><br>
<jsp:include page="SMSC Administration Guide.htm"/>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
