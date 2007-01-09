<%@ page import="ru.novosoft.smsc.jsp.util.tables.table.PagedTableBean"%>
<input type=hidden name=filterEnabled value="<%=bean.isFilterEnabled()%>">
<% if (bean instanceof PagedTableBean) { %>
<input type=hidden name=startPosition id="startPosition" value="<%=bean.getStartPosition()%>">
<%@ include file="/WEB-INF/inc/simple_table.jsp" %>
<%   if (bean.isFilterEnabled()) { %>
<%@ include file="/WEB-INF/inc/navbar.jsp" %>
<%   } else { %>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
<%   } %>
<% } else { %>
Bean must extends PagedTableBean
<% } %>