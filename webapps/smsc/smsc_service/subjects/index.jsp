<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.SubjectQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter"%>
<h4>Subjects</h4>
<a href="filter.jsp">Filter</a>
<%
  String sort = request.getParameter("sort");
  if (sort == null)
    sort = "Name";
  Vector sortOrder = new Vector();
  sortOrder.add(sort);

  SubjectFilter filter = (SubjectFilter) session.getAttribute("subject_filter");
  if (filter == null)
    filter = new SubjectFilter();

  Integer pagesizeI = (Integer) session.getAttribute("subject_page_size");
  int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();

	int startPosition = getIntegerParam(request, "startPosition");
	if (startPosition == Integer.MIN_VALUE)
		startPosition = 0;

  showTable(out, smsc.getSubjects().query(new SubjectQuery(pagesize, filter, sortOrder, startPosition)));

  if (startPosition > 0)
  {%><a href="?sort=<%=sort%>&startPosition=<%=startPosition - pagesize%>">prev</a><%}
  else
  {%>prev<%}
%>
<a href="?sort=<%=sort%>&startPosition=<%=startPosition + pagesize%>">next</a>
<%@ include file="/common/footer.jsp"%>