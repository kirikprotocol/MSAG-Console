<%@ include file="/WEB-INF/inc/page_menu.jsp"%><%@
    include file="/WEB-INF/inc/buttons.jsp"%>
<table height="100%" cellspacing="0" cellpadding="0" class="main_table"><tr>
<td width="30" background="/images/smsc_03.jpg" rowspan="3"></td>
<td bgcolor="#F3F0E3" style="position: relative; top:0; left:0;">
<!-- background  -->
<table cellspacing="0" cellpadding="0" height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
<tr><td width="12px" background="/images/smsc_24.jpg"></td><td>&nbsp;</td><td width="12px" background="/images/smsc_26.jpg"></td></tr>
</table> */%>
<!-- "SMS Service Center" header -->
<table cellspacing="0" cellpadding="0" border="0" ><tr>
  <td width="236"><IMG SRC="/images/smsc_04.jpg" WIDTH="236" HEIGHT="45" ALT=""></td>
  <td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;"><%=getLocString("common.version")%></span></td>
  <td width="12" background="/images/smsc_06.jpg"></td></tr>
</table>
<!-- main menu -->
<table width="100%" id="MAIN_MENU_TABLE" border="0"  cellspacing="0" cellpadding="0" class="main_menu"><tr>
  <td background="/images/smsc_14.jpg" width="26"></td>
  <td background="/images/smsc_14.jpg" width="100%">&nbsp;</td><%
  if (request.getUserPrincipal() != null) {%>
    <td background="/images/smsc_14.jpg"><a ID="MENU0_LOGOUT" href="logout.jsp"><%=getLocString("menu.logout")%></a></td><%
  }%>
  <td background="/images/smsc_14.jpg" width="52"></td></tr>
</table>
<span id="MENU0_NONE"></span>
<script>document.getElementById('MENU0_NONE').className = 'submenu_sel';</script>
