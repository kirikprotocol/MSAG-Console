<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
   ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	 TITLE=getLocString("smsquiz.title");
   int beanResult = bean.process(request);
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>

<%
    page_menu_begin(out);
    page_menu_space(out);
    page_menu_button(session, out, "mbStart", "common.buttons.start", "smsquiz.start");
    page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsquiz.stop");
    page_menu_end(out);
%>



<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>