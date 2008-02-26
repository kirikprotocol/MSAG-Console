<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.fraud.FraudConfigBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("fraud.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<div class=content>
  <div class=page_subtitle><%=getLocString("fraud.policy")%></div>
  <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
  <input type=hidden name=sort id=sort value="<%=bean.getSort()%>">
  <input type=hidden name=edit id=edit>
  <table class="list">
    <col width="1%">
    <col width="99%">
    <tr class="row0">
      <td nowrap><%=getLocString("fraud.enableCheck")%>:</td>
      <td align=left><input class=check type=checkbox name=enableCheck id=enableCheck <%=bean.isEnableCheck() ? "checked" : ""%>></td>
    </tr>
    <tr class="row1">
      <td nowrap><%=getLocString("fraud.enableReject")%>:</td>
      <td align=left><input class=check type=checkbox name=enableReject id=enableReject <%=bean.isEnableReject() ? "checked" : ""%>></td>
    </tr>
    <tr class="row0">
      <td nowrap><%=getLocString("fraud.tail")%>:</td>
      <td align=left><input class=txt name=tail id=tail value="<%=bean.getTail() == null ? "" : bean.getTail()%>" validation="int_range" range_min="0" range_max="10" onkeyup="resetValidation()"></td>
    </tr>
  </table>
</div>


<div class=content>
  <div class=page_subtitle><%=getLocString("fraud.whitelist")%></div>
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
        <a href="javascript:setSort('msc')" <%=bean.getSort().endsWith("msc") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.mscAddress")%>"><%=getLocString("common.sortmodes.mscAddress")%></a>
      </th>
      <%int posIdx=0;%>
      <% for (Iterator iter = bean.getRows().iterator(); iter.hasNext();) { %>
      <% String msc = (String)iter.next(); %>
      <tr class=row<%=(posIdx++) % 2%>>
        <td>
          <input class=check type=checkbox name=checked value="<%=msc%>" <%=bean.isMscChecked(msc) ? "checked" : ""%>>
        </td>
        <td><%=StringEncoderDecoder.encode(msc)%></td>
      </tr>
      <% } %>
    </tr>
  </table>
  <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbDelete", "fraud.delete.msc", "fraud.delete.msc");
  page_menu_space(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.save");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset");
  page_menu_end(out);
%>

<div class=content>
  <div class=page_subtitle><%=getLocString("fraud.add.msc")%></div>
  <table class=list cellspacing=1 width="100%">
      <tr class=row0>
        <td width="1%">
          <input class=txt type="text" name="newMsc" size=15 maxlength=15 value="<%=bean.getNewMsc()%>">
        </td>
        <td width="1%"><%button(out, "/images/but_add.gif", "mbAdd", "Add", "Add");%></td>
        <td width="98%">&nbsp;</td>
      </tr>
    </table>
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>