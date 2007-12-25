<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page onLoad="tableTag_checkChecks();">
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">service.add.title</c:when>
            <c:otherwise>service.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">

    </jsp:attribute>

    <jsp:body>
        <sm-ep:properties title="service.edit.properties.service_info">
            <c:if test="${!bean.add}"><input type="hidden" name="id" id="id" value="${fn:escapeXml(bean.id)}"></c:if>
            <c:if test="${!param.add}">
            <c:choose>
                <c:when test="${bean.editChild}">
                    <sm-ep:txt  title="service.edit.txt.id" name="parentId" readonly ="true" validation="nonEmpty"/>
                </c:when>
                <c:otherwise>
                    <sm-ep:txt  title="service.edit.txt.id" name="editId" readonly ="true" validation="nonEmpty"/>
                </c:otherwise>
            </c:choose>

            </c:if>
            <sm-ep:txt title="service.edit.txt.name" name="name" validation="nonEmpty"/>
            <sm-ep:txtBox title="service.edit.txtbox.description" cols="0" rows="0" name="description"/>
        </sm-ep:properties>

        <div class=page_subtitle>&nbsp;</div>
        <sm-pm:menu>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbSave" value="service.edit.item.mbsave.value" title="service.edit.item.mbsave.title"/>
            </c:if>
            <c:if test="${param.add}">
                <sm-pm:item name="mbSave" value="service.edit.item.mbsave2.value" title="service.edit.item.mbsave2.title"/>
            </c:if>
            <sm-pm:item name="mbCancel" value="service.edit.item.mbcancel.value" title="service.edit.item.mbcancel.title" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
        <c:if test="${!param.add}">
            <div class=page_subtitle>&nbsp;</div>
            <script type = "text/javascript">
               function openJedit(id,action)
               {
                 if (window.jEdit) {
                   alert('Such window already exist!');
//                   alert('Window already exist! window.jEdit.openjEditView(action,id)' + action + "|" + id);
                   window.jEdit.openjEditView(action,id);
                 } else if (opener && opener.jEdit && !opener.closed) {
                     alert('Such Window already exist!2');
//                   alert('opener.jEdit.openjEditView(action,id)2' + action + "|" + id);
                   opener.jEdit.openjEditView(action,id);
                 } else {
//                   alert('window.open');
                   window.open("${pageContext.request.contextPath}/rules/rules/jedit.jsp?id="+id+"&action="+action,"","width=10,height=10,left="+screen.availWidth+",top="+screen.availHeight);
                 }
//                 submitOpForm();
//                 return false;
                 return true;
               }

               function assignjEditOpener(jEditwindow, marker) {
                 if (opener && !opener.closed) {
                  opener.jEdit = jEditwindow;
                  if (marker) opener.status = "<fmt:message>jEdit.started</fmt:message>";
                 } else {
                  window.jEdit = jEditwindow;
                  if (marker) window.status = "<fmt:message>jEdit.started</fmt:message>";
                 }
               }

               function closejEditWindow() {
                 var jEditOpener;
                 if (opener && !opener.closed) {
                   jEditOpener = opener;
                 } else {
                   jEditOpener = window;
                 }
                 jEditOpener.jEdit.close();
                 jEditOpener.status = "<fmt:message>jEdit.stopped</fmt:message>";
                 jEditOpener.jEdit = null;
               }

               function jEditStarting() {
                 if (opener && !opener.closed) {
                   opener.status = "<fmt:message>jEdit.starting</fmt:message>";
                 } else {
                   window.status = "<fmt:message>jEdit.starting</fmt:message>";
                 }
               }

               function submit0(invokedBy) {
                 window.rulesFrame.location.href="${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.id}";
                 if (window.childW && !window.childW.closed && window.childW!=invokedBy && window.childW.submit0) {
                   //alert('window.childW.submit0(window)');
                   window.childW.submit0(window);
                 }
                 if (opener && !opener.closed && opener!=invokedBy && opener.submit0) {
                   //alert('opener.submit0(window)');
                   opener.submit0(window);
                 }
               }

               function fireRulesState(){
                  var rules = window.rulesFrame.rulesState;
                  var rulesTable = document.getElementById("rulesT");
                  var spans = rulesTable.getElementsByTagName("span");
//                  alert("CHECK1");
                  for (var i=0;i<rules.length;i++) {
                    //alert(rules[i].exists + " " + spans[i*2].id+ " " + spans[i*2+1].id);
//                    alert("CHECK2");
                    if (rules[i].exists) {
                      spans[i*3].style.display="inline";
                      spans[i*3+1].style.display="none";
                      spans[i*3+2].style.display="inline";
                     } else {
                      spans[i*3].style.display="none";
                      spans[i*3+1].style.display="inline";
                      spans[i*3+2].style.display="inline";
                     }
                     lockRuleButtons(rules[i].locked, spans[i*3].getElementsByTagName("input"));
                     lockRuleButtons(rules[i].locked, spans[i*3+1].getElementsByTagName("input"));
                     lockRuleButtons(!rules[i].locked, spans[i*3+2].getElementsByTagName("input"));
                  }
//                  alert("CHECK3");
                  return false;
               }

               function lockRuleButtons(isLocked, buttons) {
                 for (var j=0;j<buttons.length;j++)
                   if (isLocked)
                     buttons[j].disabled = true;
                   else
                     buttons[j].disabled = false;
               }
                function submitOpForm(){
                    opForm.submit();
                }
            </script>
        <iframe id="rulesFrame" onload="fireRulesState()"
          <c:choose>
              <c:when test="${bean.editChild}">
                src="${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.parentId}"
              </c:when>
              <c:otherwise>
                src="${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.editId}"
              </c:otherwise>
          </c:choose>
        width='0px' height="0px"></iframe>
            <sm-ep:properties title="service.edit.properties.service_rules" noEdit="true" id="rulesT">
              <sm:rule tname="SMPP" ttitle="service.edit.rule.transport.smpp"/>
              <sm:rule tname="HTTP" ttitle="service.edit.rule.transport.http"/>
              <sm:rule tname="MMS"  ttitle="service.edit.rule.transport.mms"/>
            </sm-ep:properties>

            <div class=page_subtitle>&nbsp;</div>
            <div class=page_subtitle><fmt:message>service.edit.label.smpp.routes_list</fmt:message></div>
            <sm:table columns="checkbox,id,active,enabled,archived,notes"
                      names="c,service.edit.table.names.id,service.edit.table.names.active,service.edit.table.names.enabled,service.edit.table.names.archived,service.edit.table.names.notes" widths="1,60,20,20,39,59"
                      child="/routing/routes" parentId="${bean.parentId}" edit="id" targetElemId="mbDelete"/>
            <div class=page_subtitle>&nbsp;</div>
            <sm-pm:menu>
                <sm-pm:item name="mbAddSmppRoute" value="service.edit.item.mbaddchild.value" title="service.edit.item.mbaddchild.title"/>
                <sm-pm:item name="mbDelete" value="service.edit.item.mbdelete.value" title="service.edit.item.mbdelete.title"
                            onclick="return deleteConfirm()" isCheckSensible="true"/>
                <sm-pm:space/>
            </sm-pm:menu>
            <div class=page_subtitle>&nbsp;</div>
            <div class=page_subtitle><fmt:message>service.edit.label.http.routes_list</fmt:message></div>
            <sm:http_route columns="checkbox,name,id,enabled,defaultRoute,transit" names="c,service.edit.table.http_route.name,service.edit.table.http_route.id,service.edit.table.http_route.enabled,service.edit.table.http_route.default,service.edit.table.http_route.transit" widths="1,60,20,20,39,59"
                           child="/routing/routes/http" parentId="${bean.parentId}" edit="name" targetElemId="mbDeleteHttpRoute" defaultItemId="mbDefaultHttpRoute"/>
            <div class=page_subtitle>&nbsp;</div>
            <sm-pm:menu>
                <sm-pm:item name="mbAddHttpRoute" value="service.edit.item.mbaddchild.value" title="service.edit.item.mbaddchild.title"/>
                <sm-pm:item name="mbDeleteHttpRoute" value="service.edit.item.mbdelete.value" title="service.edit.item.mbdelete.title"
                           onclick="return checkDefaultRoute()" isCheckSensible="true"/>
                <sm-pm:space/>
                <sm-pm:item name="mbDefaultHttpRoute" value="service.edit.item.mbsetdefaulthttproute.value" title="service.edit.item.mbsetdefaulthttproute.title"
                           onclick="return checkCount()" isCheckSensible="true"/>
            </sm-pm:menu>
        </c:if>
    </jsp:body>
</sm:page>