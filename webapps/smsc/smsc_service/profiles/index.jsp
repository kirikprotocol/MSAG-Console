<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.ProfileQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.ProfileFilter"%>
<h4>Profiles</h4>
<%
  String sort = request.getParameter("sort");
  if (sort == null)
    sort = "Mask";
  Vector sortOrder = new Vector();
  sortOrder.add(sort);

  ProfileFilter filter = (ProfileFilter) session.getAttribute("profile_filter");
  if (filter == null)
    filter = new ProfileFilter();

  Integer pagesizeI = (Integer) session.getAttribute("profile_page_size");
  int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();

	int startPosition = getIntegerParam(request, "startPosition");
	if (startPosition == Integer.MIN_VALUE)
		startPosition = 0;

  showTable(out, smsc.queryProfiles(new ProfileQuery(pagesize, filter, sortOrder, startPosition)));
  
  if (startPosition > 0)
  {%><a href="?sort=<%=sort%>&startPosition=<%=startPosition - pagesize%>">prev</a><%}
  else
  {%>prev<%}
%>
<a href="?sort=<%=sort%>&startPosition=<%=startPosition + pagesize%>">next</a>
<%@ include file="/sketches/footer.jsp"%>