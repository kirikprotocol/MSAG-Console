<%@
 tag body-content="scriptless"              %><%@
 attribute name="submenuId" required="true" %><%@
 attribute name="menuId"    required="true" %><%@
 attribute name="name"      required="true" %>
<td width='1%' style='position:relative; top:0; left:0;'
  onMouseOver='${submenuId}.runtimeStyle.visibility="visible";
               document.all.${submenuId}_h.runtimeStyle.visibility="visible";
               document.all.${submenuId}_h.runtimeStyle.width=${submenuId}.offsetWidth;
               document.all.${submenuId}_h.runtimeStyle.height=${submenuId}.offsetHeight;'
  onMouseOut='${submenuId}.runtimeStyle.visibility="hidden";
              document.all.${submenuId}_h.runtimeStyle.visibility="hidden";'>
<a ID="${menuId}">${name}</a>
<iframe id="${submenuId}_h" class=hiddenLayer src='/images/blank.html' width='100%' height='0px'></iframe>
<table class=submenu cellspacing=0 cellpadding=0 id="${submenuId}"
  onMouseOver='${submenuId}.runtimeStyle.visibility="visible";
               document.all.${submenuId}_h.runtimeStyle.visibility="visible";
               document.all.${submenuId}_h.runtimeStyle.width=${submenuId}.offsetWidth;
               document.all.${submenuId}_h.runtimeStyle.height=${submenuId}.offsetHeight;'
  onMouseOut='${submenuId}.runtimeStyle.visibility="hidden";
              document.all.${submenuId}_h.runtimeStyle.visibility="hidden";'>
<jsp:doBody/>
</table>
</td>