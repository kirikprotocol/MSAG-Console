<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="subjects.index.title">
    <jsp:attribute name="menu">
    </jsp:attribute>
    <jsp:body>

        <script>
        function changeTransportId() {
              var transport = getElementByIdUni("transportId").options[getElementByIdUni("transportId").selectedIndex].value;
              var opForm = getElementByIdUni("opForm");
              if (location.href.indexOf("transportId")!=-1) {
                var action = location.href.substring(0,location.href.indexOf("transportId")-1);
                opForm.action=action;
              }
              opForm.submit();
              return true;
        }
        </script>
        
        <table class=properties_list cellspacing=0 cellpadding=0>
            <col width="15%">
            <col width="100%">
            <c:set var="prop_rowN" value="0" scope="request"/>
            <sm-ep:list title="subjects.index.list.transportid" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </table>
        <br>
        <c:choose>
            <c:when test="${bean.transportId == 1}">
                <div class="page_subtitle"><fmt:message>subjects.index.label.smpp_subject</fmt:message></div>
                <sm:table columns="checkbox,name,notes" names="c,subjects.index.table.names.name,subjects.index.table.names.notes" widths="1,49,50" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAdd" value="subjects.index.item.mbadd.value" title="subjects.index.item.mbadd.title"/>
                    <sm-pm:item name="mbDelete" value="subjects.index.item.mbdelete.value" title="subjects.index.item.mbdelete.title"
                                onclick="return deleteConfirm()" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>

            </c:when>
            <c:when test="${bean.transportId == 2}">
                <div class="page_subtitle"><fmt:message>subjects.index.label.http_abonents</fmt:message></div>
                <sm:subject columns="checkbox,name" names="c,subjects.index.subject.names.name" widths="1,99" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAddHttpSubj" value="subjects.index.item.mbaddhttpsubj.value" title="subjects.index.item.mbaddhttpsubj.title"/>
                    <sm-pm:item name="mbDeleteHttpSubj" value="subjects.index.item.mbdeletehttpsubj.value" title="subjects.index.item.mbdeletehttpsubj.title"
                                onclick="return deleteConfirm()" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>
                <div class=page_subtitle>&nbsp;</div>

                <div class="page_subtitle"><fmt:message>subjects.index.label.http_subjects</fmt:message></div>
                <sm:site columns="checkbox,name" names="c,subjects.index.site.names.name" widths="1,99" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAddHttpSite" value="subjects.index.item.mbadd.value" title="subjects.index.item.mbadd.title"/>
                    <sm-pm:item name="mbDeleteHttpSite" value="subjects.index.item.mbdelete.value" title="subjects.index.item.mbdelete.title"
                                onclick="return deleteConfirm()" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>

            </c:when>
            <c:when test="${bean.transportId == 3}">
            </c:when>
        </c:choose>
    </jsp:body>
</sm:page>