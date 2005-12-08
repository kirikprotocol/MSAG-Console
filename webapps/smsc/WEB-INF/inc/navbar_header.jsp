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
%>
<script>
function navigatePage(pageNum)
{
	document.getElementById('startPosition').value = pageNum*<%=bean.getPageSize()%>;
    if(document.getElementById('runQuery') != null){
        document.getElementById('runQuery').value = false;
    }
    opForm.submit();
	return false;
}
</script>
<table class=navbar cellspacing=1 cellpadding=0>
<tr>
<td class=first><a href="javascript:navigatePage(0)"><img src="/images/nav_first.gif" width="12" height="11" alt="<%=getLocString("navbar.firstPage")%>"></a></td>
<%if(pageCurrent > 0){%>
<td class=prev><a href="javascript:navigatePage(<%=pageCurrent-1%>)"><img src="/images/nav_prev.gif" width="12" height="11" alt="<%=getLocString("navbar.prevPage")%>"></a></td>
<%}
for(int pageNum=pageFirst;pageNum<=pageLast;pageNum++)
{%>
<td class="<%=pageNum==pageCurrent ? "current":"page"%>" ><a href="javascript:navigatePage(<%=pageNum%>)" title="<%=getLocString("navbar.pagePre") + pageNum + 1%>"><%=pageNum + 1%></a></td>
<%}%>
<%if(pageCurrent+1 < pageTotal){%>
<td class=next><a href="javascript:navigatePage(<%=pageCurrent+1%>)"><img src="/images/nav_next.gif" width="12" height="11" alt="<%=getLocString("navbar.nextPage")%>"></a></td>
<%}%>
<td class=last><a href="javascript:navigatePage(<%=pageTotal > 0 ? pageTotal-1 : 0%>)"><img src="/images/nav_last.gif" width="12" height="11" alt="<%=getLocString("navbar.lastPage")%>"></a></td>
<td class=total><%= getLocString("common.sortmodes.total") + ": " + itemsTotal + " " + getLocString("common.util.items")%></td>
