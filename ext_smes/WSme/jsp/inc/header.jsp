<%
WSmeContext.init(appContext);
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