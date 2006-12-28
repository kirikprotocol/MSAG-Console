<%@ page import="ru.novosoft.smsc.jsp.util.tables.table.PagedTableBean"%>
<% if (bean instanceof PagedTableBean) { %>
<input type=hidden name=startPosition id="startPosition" value="<%=bean.getStartPosition()%>">
<%@ include file="/WEB-INF/inc/simple_table.jsp" %>
<%   if (bean.isShowFilter()) { %>
<%@ include file="/WEB-INF/inc/navbar.jsp" %>
<%   } else { %>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
<%   } %>
<% } else { %>
Bean must extends PagedTableBean
<% } %>