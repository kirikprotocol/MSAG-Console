<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.SubjectQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter"%>
<h4>Subjects</h4>
<%
  String sort = request.getParameter("sort");
  if (sort == null)
    sort = "Name";
  Vector sortOrder = new Vector();
  sortOrder.add(sort);
  showTable(out, smsc.getSubjects().query(new SubjectQuery(10, new SubjectFilter(), sortOrder, 0)));
%>
<%@ include file="/common/footer.jsp"%>