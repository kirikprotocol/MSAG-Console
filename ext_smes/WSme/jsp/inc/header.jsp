<%@ page import="ru.novosoft.smsc.wsme.WSmeContext,
                 ru.novosoft.smsc.jsp.SMSCAppContext"%><%
//WSmeContext.init((SMSCAppContext) request.getAttribute("appContext"));
%>
<script language="JScript">
function noValidationSubmit(buttonElem)
{
  document.getElementById('jbutton').value = buttonElem.jbuttonValue;
  document.getElementById('jbutton').name  = buttonElem.jbuttonName;
  opForm.submit();
  return false;
}
</script>