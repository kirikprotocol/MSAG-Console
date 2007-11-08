</td>
<td height="1px" width="12px" background="/images/smsc_26.jpg"></td>
</tr>
</table>
</td>

<td width="30" background="/images/smsc_07.jpg" rowspan="3"></td></tr>
<tr height="100%"><td style="position:relative;">
<%--<!-- background -->--%>
<table cellspacing="0" cellpadding="0" height="100%" >
  <tr><td height="1px" width="12px" background="/images/smsc_24.jpg"></td><td height="1px"><img height="0px" width="1px"></td><td height="1px" width="12px" background="/images/smsc_26.jpg"></td></tr>
</table>
</td></tr>
<tr><td>
<table cellpadding="0" cellspacing="0"><tr>
<td width="12" background="/images/smsc_30.jpg"></td><td align="right" bgcolor="#161E49"><span style="color: #6F77A3;"><%=getLocString("common.copyright")%></span></td><td width="12" background="/images/smsc_32.jpg"></td></tr>
</table>
</td></tr>
</table>
</body>
<script>
<% if (isServiceStatusNeeded || (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)) { %>
if (serviceStatusDataSource.hasObservers())
  window.setTimeout(refreshServiceStatuses, 10);
if (smeStatusDataSource.hasObservers())
  window.setTimeout(refreshSmeStatuses, 10000);
if (smscStatusDataSource.hasObservers())
  window.setTimeout(refreshSmscStatuses, 10000);
<%}%>
selectFirstTextInput();
</script>
</html>