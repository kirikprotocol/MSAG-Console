<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 java.util.*,
                 ru.novosoft.smsc.emailsme.backend.ProfilesDataItem,
                 ru.novosoft.smsc.emailsme.beans.Profiles,
                 java.net.URLEncoder"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Profiles" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
  int beanResult = bean.process(request);
  switch (beanResult)
  {
    case Profiles.RESULT_EDIT:
      response.sendRedirect("edit.jsp?addr=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case Profiles.RESULT_ADD:
      response.sendRedirect("edit.jsp?create=true");
      return;
    case Profiles.RESULT_DONE:
      response.sendRedirect("profiles.jsp");
      return;
  }
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<%
  if (beanResult != Profiles.RESULT_ERROR)
  {
    %>
    <div class=content>
    <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
    <input type=hidden name=edit>
    <input type=hidden name=sort>
    <script>
    function setSort(sorting)
    {
      if (sorting == "<%=bean.getSort()%>")
        opForm.sort.value = "-<%=bean.getSort()%>";
      else
        opForm.sort.value = sorting;
      opForm.submit();
      return false;
    }
    </script>
    <table class=list cellspacing=0>
    <thead>
    <tr>
      <th width="1%" >&nbsp;</th>
      <th width="25%"><a href="#" <%=bean.getSort().endsWith("addr") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by address" onclick='return setSort("addr")'>address</a></th>
      <th width="25%"><a href="#" <%=bean.getSort().endsWith("userid") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by user id" onclick='return setSort("userid")'>user ID</th>
      <th width="25%"><a href="#" <%=bean.getSort().endsWith("dayLimit") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by day limit" onclick='return setSort("dayLimit")'>day limit</th>
      <th width="25%"><a href="#" <%=bean.getSort().endsWith("forward") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by forward address" onclick='return setSort("forward")'>forward address</th>
    </tr>
    </thead>
    <tbody>
    <%
      int row = 0;
      QueryResultSet results = bean.getResultSet();
      for (Iterator i = results.iterator(); i.hasNext();) {
        ProfilesDataItem item = (ProfilesDataItem) i.next();
        String addrEnc = StringEncoderDecoder.encode(item.getAddr());
        String userIdEnc = StringEncoderDecoder.encode(item.getUserid());
        String forwardEnc = StringEncoderDecoder.encode(item.getForward());
        %><tr class=row<%=row++&1%>>
          <td><input class=check type=checkbox name=checked value="<%=addrEnc%>" onclick="checkCheckboxesForMbDeleteButton();" <%=bean.isChecked(item.getAddr()) ? "checked" : ""%>></td>
          <td><a href="#" title="Edit profile" clickValue="<%=addrEnc%>" onClick="return editSomething(this.clickValue);"><%=addrEnc%></a></td>
          <td><%=userIdEnc%></td>
          <td><%=item.getDayLimit()%></td>
          <td><%=forwardEnc%></td>
        </tr><%
      }
    %>
    </tbody>
    </table>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
    </div><%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAdd",    "Add",    "Add new profile");
    page_menu_confirm_button(session, out, "mbDelete", "Delete", "Delete checked profile(s)", "Are you sure to delete all checked profile(s)?");
    page_menu_space(out);
    page_menu_end(out);
    %>
    <script>
    checkCheckboxesForMbDeleteButton();
    </script><%
  }%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
