<%@tag body-content="scriptless"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<!------ page menu ------>
<%@ variable name-given="page_menu_delimiter_needed" scope="NESTED" %>
<c:set var="page_menu_delimiter_needed" value="false" scope="request"/>
<div class=page_menu>
<table class=page_menu cellpadding=0px cellspacing=0px>
<tr>
  <td width=37px>&nbsp;</td>
  <jsp:doBody/>
  <td width=37px>&nbsp;</td>
</tr>
</table>

<table class=page_menu_background cellspacing=0px cellpadding=0px>
<tr>
  <td width=80px background="/images/smsc_20.jpg">&nbsp;</td>
  <td background="/images/smsc_21.jpg">&nbsp;</td>
  <td width=80px background="/images/smsc_23.jpg">&nbsp;</td>
</tr>
</table>
</div>
<!------ page menu end ------>