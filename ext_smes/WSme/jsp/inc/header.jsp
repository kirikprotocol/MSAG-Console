<%@ page import="ru.novosoft.smsc.wsme.WSmeContext,
                 ru.novosoft.smsc.jsp.SMSCAppContext"%><%
//WSmeContext.init((SMSCAppContext) request.getAttribute("appContext"));
%>
<script language="JScript">
function noValidationSubmit(buttonElem)
{
  document.all.jbutton.value = buttonElem.jbuttonValue;
  document.all.jbutton.name  = buttonElem.jbuttonName;
  opForm.submit();
  return false;
}
</script>