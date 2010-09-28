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

               function submit0(invokedBy)
               {
//                   alert("edit:submit0():rulesFrame=" + rulesFrame);
//o                 window.rulesFrame.location.href="${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.id}";
//                 getElementByIdUni("rulesFrame").location.href = "${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.id}";
                 getElementByIdUni("rulesFrame").src = "${pageContext.request.contextPath}/rules/rules/rules.jsp?id=${bean.id}";
                 if (window.childW && !window.childW.closed && window.childW!=invokedBy && window.childW.submit0) {
                   alert('edit:window.childW.submit0(window)');
                   window.childW.submit0(window);
                 }
                 if (opener && !opener.closed && opener!=invokedBy && opener.submit0) {
                   alert('edit:opener.submit0(window)');
                   opener.submit0(window);
                 }
               }

               function lockRuleButtons_(isLocked, buttons) {
                 for (var j=0;j<buttons.length;j++)
                   if (isLocked)
                     buttons[j].disabled = true;
                   else
                     buttons[j].disabled = false;
               }
                function submitOpForm(){
                    opForm.submit();
                }

                function isMSIE()
                {
                  return navigator.appName == "Microsoft Internet Explorer";
                }

               function fireRulesState(){
                 var fr = getElementByIdUni("rulesFrame");
//                  alert("rulesFrame.contentDocument=" + rulesFrame.contentDocument);
                 var rules;
                 if( isMSIE() ){
                     rules = window.rulesFrame.rulesState;
                 } else{
                  rules = getRules();
                }
                  var rulesTable = document.getElementById("rulesT");
                  var spans = rulesTable.getElementsByTagName("span");
//                  alert("CHECK1");
                  for (var i=0;i<rules.length;i++) {
//                    alert("rules[i]=" +rules[i].exists + " | " + rules[i].locked );
                    if (rules[i].exists) {
                      spans[i*3].style.display="inline";
                      spans[i*3+1].style.display="none";
                      spans[i*3+2].style.display="inline";
                     } else {
                      spans[i*3].style.display="none";
                      spans[i*3+1].style.display="inline";
                      spans[i*3+2].style.display="inline";
                     }
                     if( spans[i*3].style.display!="none" ){
                         lockRuleButtons( rules[i].locked, spans[i*3].getElementsByTagName("input") );
                     }
                     if( spans[i*3+1].style.display!="none" ){
                        lockRuleButtons( rules[i].locked, spans[i*3+1].getElementsByTagName("input") );
                     }
                     if( spans[i*3+2].style.display!="none"){
                         lockRuleButtons( !rules[i].locked, spans[i*3+2].getElementsByTagName("input") );
                     }
                  }
//                  alert("CHECK3");
                  return false;
               }

               function unlock()
               {
                 alert("unlock");
               }

               function lockRuleButtons(isLocked, buttons) {
                 for (var j=0;j<buttons.length;j++){
//                   alert( "buttons[j]=" + buttons[j]);
                   if (isLocked){
                     buttons[j].disabled = true;
                     buttons[j].style.color = "gray";
                     buttons[j].style.cursor = "none";
                   }
                   else{
                     buttons[j].disabled = false;
                     buttons[j].style.color = "black";
                     buttons[j].style.cursor = "pointer";
                   }
                 }
               }

                function getRules()
                {
                    var x=document.getElementById("rulesFrame").contentDocument;
                    var arr = x.getElementsByTagName("*");
                    var smppRuleStateE;
                    var smppRuleStateL;
                    var httpRuleStateE;
                    var httpRuleStateL;
                    var mmsRuleStateE;
                    var mmsRuleStateL;
                    for(var i=0; i<arr.length; i++){
                        if( arr[i].type != 'undefined' && arr[i].type == "hidden" ){
    //                        alert("HIDDEN :" + arr[i].id + " value = " + arr[i].value );
                            if( arr[i].id=='smppRuleStateE' ){
                                smppRuleStateE = tf(arr[i].value);
                            } else if( arr[i].id=='smppRuleStateL' ){
                                smppRuleStateL = tf(arr[i].value);
                            } else if( arr[i].id=='httpRuleStateE' ){
                                httpRuleStateE = tf(arr[i].value);
                            } else if( arr[i].id=='httpRuleStateL' ){
                                httpRuleStateL = tf(arr[i].value);
                            } else if( arr[i].id=='mmsRuleStateE' ){
                                mmsRuleStateE = tf(arr[i].value);
                            } else if( arr[i].id=='mmsRuleStateL' ){
                                mmsRuleStateL = tf(arr[i].value);
                            }
                        }
                    }
                    var rules = new Array();
                    rules[0]={exists:smppRuleStateE,locked:smppRuleStateL};
                    rules[1]={exists:httpRuleStateE,locked:httpRuleStateL};
                    rules[2]={exists:mmsRuleStateE ,locked:mmsRuleStateL};
                    return rules;
                }

                function tf(val)
                {
                    return val=='true'? true: false;
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
          width='0px' height="0px">
        </iframe>

            <sm-ep:properties title="service.edit.properties.service_rules" noEdit="true" id="rulesT">
              <sm:rule tname="SMPP" ttitle="service.edit.rule.transport.smpp"/>
              <sm:rule tname="HTTP" ttitle="service.edit.rule.transport.http"/>
              <!-- Hide MMS -->
              <!--sm:rule tname="MMS"  ttitle="service.edit.rule.transport.mms"/-->
            </sm-ep:properties>

            <div class=page_subtitle>&nbsp;</div>
            <div class=page_subtitle><fmt:message>service.edit.label.smpp.routes_list</fmt:message></div>
            <sm:table columns="checkbox,id,enabled,transit,saa,notes"
                      names="c,service.edit.table.names.id,service.edit.table.names.enabled,
                             service.edit.table.names.transit,service.edit.table.names.statistic_saa,
                             service.edit.table.names.notes"
                      widths="1,60,20,20,39,59" child="/routing/routes" parentId="${bean.parentId}" edit="id"
                      targetElemId="mbDelete" ali="center" exc="id"/>
            <div class=page_subtitle>&nbsp;</div>
            <sm-pm:menu>
                <sm-pm:item name="mbAddSmppRoute" value="service.edit.item.mbaddchild.value" title="service.edit.item.mbaddchild.title"/>
                <sm-pm:item name="mbDelete" value="service.edit.item.mbdelete.value" title="service.edit.item.mbdelete.title"
                            onclick="return deleteConfirm()" isCheckSensible="true"/>
                <sm-pm:space/>
            </sm-pm:menu>
            <div class=page_subtitle>&nbsp;</div>
            <div class=page_subtitle><fmt:message>service.edit.label.http.routes_list</fmt:message></div>
            <sm:http_route columns="checkbox,name,id,enabled,defaultRoute,transit,saa"
                           names="c,service.edit.table.http_route.name,service.edit.table.http_route.id,
                                  service.edit.table.http_route.enabled,service.edit.table.http_route.default,
                                  service.edit.table.http_route.transit, service.edit.table.names.statistic_saa"
                           widths="1,60,20,20,39,59" child="/routing/routes/http" parentId="${bean.parentId}"
                           edit="name" targetElemId="mbDeleteHttpRoute" defaultItemId="mbDefaultHttpRoute"
                           ali="center" exc="name"/>
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
