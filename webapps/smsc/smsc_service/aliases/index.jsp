<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.AliasQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter"%>
<h4>Aliases</h4>
<%
  String sort = request.getParameter("sort");
  if (sort == null)
    sort = "Alias";
  Vector sortOrder = new Vector();
  sortOrder.add(sort);
  showTable(out, smsc.getAliases().query(new AliasQuery(10, new AliasFilter(), sortOrder, 0)));
%>
<%@ include file="/common/footer.jsp"%>