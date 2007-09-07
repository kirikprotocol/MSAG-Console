<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.BlackNickIndex,
                 ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.smsc.IndexBean,
                 ru.novosoft.smsc.jsp.util.tables.impl.blacknick.BlackNickDataItem"%>
<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<jsp:useBean id="bean" scope="request" class="ru.novosoft.smsc.jsp.smsc.profiles.BlackNickIndex"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("profiles.black.nick.list");
    MENU0_SELECTION = "MENU0_MSCMAN";

    int beanResult = bean.process(request);
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<div class=content>
    <input type=hidden name=sort value="<%=bean.getSort()%>">
    <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
    <%=getLocString("profiles.black.nick.prefix")%> <input class=txt name=filter value="<%=bean.getFilter()%>">
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbQuery", "common.buttons.query", "common.buttons.query");
    page_menu_button(session, out, "mbDelete", "common.buttons.delete", "common.buttons.delete");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content>
  <script>
    function setSort(sorting) {
      if (sorting == "<%=bean.getSort()%>")
        opForm.sort.value = "-<%=bean.getSort()%>";
      else
        opForm.sort.value = sorting;
      opForm.submit();
      return false;
    }
  </script>
  <table class=list>
    <col width="1%">
    <col width="99%">
    <tr>
      <th><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
      <th>
        <a href="javascript:setSort('nick')" <%=bean.getSort().endsWith("nick") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.nickHint")%>"><%=getLocString("common.sortmodes.nick")%></a>
      </th>
    </tr>
    <%int posIdx=0;%>
    <%for (int i=bean.getStartPositionInt(); i<bean.getRs().size() && i < bean.getStartPositionInt() + bean.getPageSizeInt(); i++) {%>
      <% String nick = (String)((BlackNickDataItem)bean.getRs().get(i)).getValue("nick");%>
      <tr class=row<%=(posIdx++) % 2%>>
        <td>
          <input class=check type=checkbox name=checked value="<%=nick%>" <%=bean.isNickChecked(nick) ? "checked" : ""%> onclick="checkCheckboxes();">
        </td>
        <td><%=nick%></td>
      </tr>
    <%}%>
  </table>
  <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>

  <div class=page_subtitle><%=getLocString("profiles.new.black.nick")%></div>
    <table class=list cellspacing=1 width="100%">
      <tr class=row0>
        <td width="1%">
          <input class=txt type="text" name="newNick" size=21 maxlength=21>
        </td>
        <td width="1%"><%button(out, "/images/but_add.gif", "mbAdd", "Add", "Add");%></td>
        <td width="98%">&nbsp;</td>
      </tr>
    </table>
  </div>

</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
