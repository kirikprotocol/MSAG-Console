<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.AliasQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter"%>
<h4>Aliases</h4>
<a href="filter.jsp">Filter</a>
<%
	String sort = request.getParameter("sort");
	if (sort == null)
		sort = "Alias";
	Vector sortOrder = new Vector();
	sortOrder.add(sort);
	
	AliasFilter filter = (AliasFilter) session.getAttribute("alias_filter");
	if (filter == null)
	{
		filter = new AliasFilter();
	}
	
	Integer pagesizeI = (Integer) session.getAttribute("alias_page_size");
	int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();
	
	showTable(out, smsc.getAliases().query(new AliasQuery(pagesize, filter, sortOrder, 0)));
%>
<%@ include file="/common/footer.jsp"%>