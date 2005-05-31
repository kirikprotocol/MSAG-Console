<%@ page import="java.io.PrintWriter"%>
<%{%><%--

try {
%>
total size: <%=bean.getTotalSizeInt()%><br>
start position: <%=bean.getStartPositionInt()%><br>
page size: <%=bean.getPageSizeInt()%><br>
<%
} catch (Throwable e) {
  %><pre style="color:red;"><%
  e.printStackTrace(new PrintWriter(out));
  %></pre><%
}
--%><%
int itemsTotal = bean.getTotalSizeInt(); // 31
int pageCurrent = bean.getStartPositionInt() / bean.getPageSizeInt(); // 1
int pageFirst = (pageCurrent -4) < 0 ? 0 : (pageCurrent -4); // 0
int pageTotal = (itemsTotal / bean.getPageSizeInt()) + ((itemsTotal % bean.getPageSizeInt()) > 0 ? 1 : 0); // 2
int pageLast = (pageCurrent +4) > (pageTotal-1) ? (pageTotal-1) : (pageCurrent +4);
String jscript = "navigatePage";
%>
<script>
function navigatePage(pageNum)
{
	document.all.startPosition.value = pageNum*<%=bean.getPageSize()%>;
    if(document.all.runQuery != null){
        document.all.runQuery.value = false;
    }
    opForm.submit();
	return false;
}
</script>
<table class=navbar cellspacing=1 cellpadding=0>
<tr>
<td class=first onclick="return <%=jscript%>(0)" title="<%=getLocString("navbar.firstPage")%>"><a href="#">&nbsp;</a></td>
<%if(pageCurrent > 0){%>
<td class=prev onclick="return <%=jscript%>(<%=pageCurrent-1%>)" title="<%=getLocString("navbar.prevPage")%>"><a href="#">&nbsp;</a></td>
<%}
for(int pageNum=pageFirst;pageNum<=pageLast;pageNum++)
{%>
<td class=<%=pageNum==pageCurrent ? "current":"page"%> onclick="return <%=jscript%>(<%=pageNum%>)" title="<%=getLocString("navbar.pagePre") + pageNum + 1%>"><%=pageNum + 1%></td>
<%}%>
<%if(pageCurrent+1 < pageTotal){%>
<td class=next title="<%=getLocString("navbar.nextPage")%>" onclick="return <%=jscript%>(<%=pageCurrent+1%>)"><a href="#">&nbsp;</a></td>
<%}%>
<td class=last onclick="return <%=jscript%>(<%=pageTotal > 0 ? pageTotal-1 : 0%>)" title="<%=getLocString("navbar.lastPage")%>"><a href="#">&nbsp;</a></td>
<td class=total><%= getLocString("common.sortmodes.total") + ": " + itemsTotal + " " + getLocString("common.util.items")%></td>
