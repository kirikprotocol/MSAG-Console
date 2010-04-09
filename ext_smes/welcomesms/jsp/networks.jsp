<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.welcomesms.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	 TITLE=getLocString("welcomesms.title")+": "+getLocString("welcomesms.label.networks");
   int beanResult = bean.process(request);
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>

<%
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>