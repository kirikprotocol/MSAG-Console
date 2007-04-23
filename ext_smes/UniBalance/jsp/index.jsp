<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.unibalance.bean.Index,
                 ru.novosoft.smsc.unibalance.backend.SmeContext"%>
<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.unibalance.bean.Index"/>
<jsp:setProperty name="bean" property="*"/>

<%
  isServiceStatusNeeded = true;
  ServiceIDForShowStatus = SmeContext.SME_ID;
  TITLE = getLocString("unibalance.config.title");

  bean.process(request);
  int rowN=0;

  MENU0_SELECTION = "MENU0_SMSC_Reshedule";
%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  sme_menu_space(out);
  sme_menu_button(out, "mbStart",  "start",  "Start",  "Start service", !bean.isServiceOnline());
  sme_menu_button(out, "mbStop",  "stop",  "Stop",  "Stop service", bean.isServiceOnline());
	sme_menu_end(out);
%>

<input type=hidden name=tableCounter value="<%=bean.getBalanceCurrencyDescriptions().size()%>">

<script language="JavaScript">

function addParam() {
  counter = document.getElementById("tableCounter");
  i = parseInt(counter.value) + 1;
  sectionName = "<%=Index.CURRENCY_PREFIX%>" + i;
  tableElem = document.getElementById("paramTable");

  curCode = document.getElementById("newCode");
  curName = document.getElementById("newName");

  newRow = tableElem.insertRow(tableElem.rows.length - 1);
  newRow.className = "row" + (tableElem.rows.length & 1);
  newRow.id = "paramRow_" + sectionName;
  counter.value = i;

  codeInputElement = document.createElement("input");
  codeInputElement.name = sectionName + "<%=Index.CURRENCY_CODE_PREFIX%>";
  codeInputElement.value = curCode.value;
  codeInputElement.className = "txt";
  newCell = newRow.insertCell(0);
  newCell.appendChild(codeInputElement);

  nameInputElement = document.createElement("input");
  nameInputElement.name = sectionName + "<%=Index.CURRENCY_NAME_PREFIX%>";
  nameInputElement.value = curName.value;
  nameInputElement.className = "txtW";
  newCell = newRow.insertCell(1);
  newCell.appendChild(nameInputElement);

  imgElement = document.createElement("img");
  imgElement.src = "/images/but_del.gif";
  imgElement.setAttribute('sectionName', sectionName);
  imgElement.attachEvent("onclick", removeParam_Event);
  newCell = newRow.insertCell(2);
  newCell.appendChild(imgElement);

  curCode.value="";
  curName.value="";
}

function removeParam_Event() {
  delParam(event.srcElement.attributes.sectionName.nodeValue);
}

function delParam(sectionId) {
  tableElem = document.getElementById("paramTable");
  rowId = "paramRow_" + sectionId;
  rowElem = tableElem.rows(rowId);
  tableElem.deleteRow(rowElem.rowIndex);
}
</script>


<div class=content>
  <table class=properties_list cellspacing=0 cellspadding=0>
    <col width="25%">
    <col width="75%">
    <tr class=row0>
       <td><%=getLocString("unibalance.balance.response.pattern")%>:</td>
       <td><input class=txtW name=balanceResponse value="<%=bean.getBalanceResponse()%>"></td>
    </tr>
    <tr class=row1>
       <td><%=getLocString("unibalance.balance.negative.response.pattern")%>:</td>
       <td><input class=txtW name=balanceNegativeResponse value="<%=bean.getBalanceNegativeResponse()%>"></td>
    </tr>
    <tr class=row0>
       <td><%=getLocString("unibalance.balance.with.accumulator.response.pattern")%>:</td>
       <td><input class=txtW name=balanceWithAccumulatorResponse value="<%=bean.getBalanceWithAccumulatorResponse()%>"></td>
    </tr>
    <tr class=row1>
       <td><%=getLocString("unibalance.balance.with.accumulator.negative.response.pattern")%>:</td>
       <td><input class=txtW name=balanceWithAccumulatorNegativeResponse value="<%=bean.getBalanceWithAccumulatorNegativeResponse()%>"></td>
    </tr>
    <tr class=row0>
       <td><%=getLocString("unibalance.balance.wait.for.sms.response.pattern")%>:</td>
       <td><input class=txtW name=balanceWaitForSmsResponse value="<%=bean.getBalanceWaitForSmsResponse()%>"></td>
    </tr>
    <tr class=row1>
       <td><%=getLocString("unibalance.balance.error.pattern")%>:</td>
       <td><input class=txtW name=balanceError value="<%=bean.getBalanceError()%>"></td>
    </tr>
    <tr class=row0>
       <td><%=getLocString("unibalance.balance.banner.add.pattern")%>:</td>
       <td><input class=txtW name=balanceBannerAdd value="<%=bean.getBalanceBannerAdd()%>"></td>
    </tr>
  </table>
</div>

<div class=content>
    <div class=secSmsc><%=getLocString("unibalance.balance.currency.configuration")%></div>
    <table class=list cellspacing=0 cellspadding=0>
      <col width="25%">
      <col width="75%">
      <tr class=row1>
        <td><%=getLocString("unibalance.balance.currency.default")%>:</td>
        <td><input class=txtW name=balanceCurrencyDefault value="<%=bean.getBalanceCurrencyDefault()%>"></td>
      </tr>
    </table>
    <%
      startSection(out, "unibalanceCurrenciesTable", "unibalance.balance.currencies", true);
      startParams(out, "paramTable");
    %>
    <tr class=row0>
      <td nowrap><%=getLocString("unibalance.balance.currency.code")%></td>
      <td><%=getLocString("unibalance.balance.currency.name")%></td>
    </tr>
    <%
      int i = 0;
      for (i = 0; i< bean.getBalanceCurrencyDescriptions().size(); i++) {
        final Index.BalanceCurrencyDescription desc = (Index.BalanceCurrencyDescription)bean.getBalanceCurrencyDescriptions().get(i);
    %>
    <tr class=row<%=((row++) & 1)%> id=<%="paramRow_" + bean.getCurrencySectionParamName(i+1)%>>
      <td width="20%"><input class=txtW id="<%=bean.getCurrencyCodeParamName(i+1)%>" name="<%=bean.getCurrencyCodeParamName(i+1)%>" value="<%=desc.getCurrencyCode()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
      <td width="80%"><input class=txtW id="<%=bean.getCurrencyNameParamName(i+1)%>" name="<%=bean.getCurrencyNameParamName(i+1)%>" value="<%=desc.getCurrencyName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
      <td><img src="/images/but_del.gif" onclick="delParam('<%=bean.getCurrencySectionParamName(i+1)%>')" title="<%=getLocString("common.hints.delParam")%>"></td>
    </tr>
    <%
      }
    %>
     <tr class=row<%=((row++) & 1)%> id=<%="paramRow_" + bean.getCurrencySectionParamName(i+2)%>>
       <td width="20%"><input class=txtW id=<%=bean.getCurrencyCodeParamName(i+2)%> name="newCode"></td>
       <td width="80%"><input class=txtW id=<%=bean.getCurrencyNameParamName(i+2)%> name="newName"></td>
       <td><img src="/images/but_add.gif" onclick="addParam()" title="<%=getLocString("common.hints.addParam")%>"></td>
     </tr>
    <%
      finishParams(out);
      finishSection(out);
    %>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.save");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>