<hr size=0 align=center width="95%">
<table width="95%" cellpadding=1 cellspacing=1 border=0>
<tr>
  <td align=left>
    <% if (formBean.getRowsCount()>0) {
    %> <b>Total&nbsp;messages&nbsp;found&nbsp;</b><%=
      formBean.getRowsCount()
    %> <b>, displayed </b><%= firstIndex
    %><% if (firstIndex != lastIndex) {%>... <%= lastIndex%> <%};
    } else {
    %><b>No messages matched, try to process another query.</b><%
    } %>
  </td>
  <% if (formBean.getRowsCount()>0 && formBean.getRowsToDisplay()>0) {
  %><td align=right>
    <b>pages&nbsp;:&nbsp;<%
    {
      int pagesCount = formBean.getPagesCount();
      int pageIndex  = formBean.getPageIndex();
      boolean large = pagesCount > SmsViewFormBean.INDEX_PAGES_LIST_SIZE;

      int startInd = pageIndex-SmsViewFormBean.INDEX_PAGES_LIST_SIZE/2;
      startInd = (startInd>0 && large) ? startInd:0;
      int endInd = startInd+SmsViewFormBean.INDEX_PAGES_LIST_SIZE;
      endInd = (endInd<=pagesCount && large) ? endInd:pagesCount;
      startInd = endInd-SmsViewFormBean.INDEX_PAGES_LIST_SIZE;
      if (startInd<0) startInd=0;
      if (startInd > 0) %>&nbsp;...&nbsp;<%
      for (int i=startInd; i<endInd; i++) {
        if (i != pageIndex) {
          %><a href="smsview.jsp?page=<%= i%>"><%= i+1%></a>&nbsp;<%
        } else { %>[<%= i+1%>]&nbsp;<% }
      }
      if (endInd < pagesCount) %>...<%
      %>&nbsp;<%
    }
    if (formBean.isPrevEnabled()) {
    %> <a href="smsview.jsp?first=true">|&lt;</a>&nbsp;
       <a href="smsview.jsp?prev=true">&lt;&lt;</a><%
    } else {
      %>&nbsp;|&lt;&nbsp;&lt;&lt;<%
    } %>&nbsp;&nbsp;<%
    if (formBean.isNextEnabled()) {
    %> <a href="smsview.jsp?next=true">&gt;&gt;</a>&nbsp;
       <a href="smsview.jsp?last=true">&gt;|</a><%
    } else {
      %>&nbsp;&gt;&gt;&nbsp;&gt;|<%
    } %> </b>
  </td><%
  } %>
</tr>
</table>
<hr size=0 align=center width="95%">