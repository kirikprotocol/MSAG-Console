<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">centers.add.title</c:when>
            <c:otherwise>centers.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="centers.edit.item.mbsave.value" title="centers.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="centers.edit.item.mbcancel.value" title="centers.edit.item.mbcancel.title" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <script>
            function changeTransportId() {
                var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
                opForm.submit();
                return true;
            }
        </script>
        <sm-ep:properties title="">
            <sm-ep:list title="centers.edit.list.transportid" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}" valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </sm-ep:properties>
        <br>
        <br>
        <br>
        <c:choose>
            <c:when test="${bean.transportId == 1}">
                <c:set var="smes" value="${fn:join(bean.smes, ',')}"/>
                <sm-ep:properties title="centers.edit.properties.smpp_info">
                    <br>
                    <sm-ep:txt title="sp.edit.txt.id" name="id" maxlength="15" validation="id"/>
                    <sm-ep:txt title="centers.edit.txt.bindsystemid" name="bindSystemId" maxlength="15" validation="id"/>
                    <sm-ep:txt title="centers.edit.txt.bindpassword" name="bindPassword"/>
                    <sm-ep:txt title="centers.edit.txt.systemtype" name="systemType"/>
                    <sm-ep:txt title="centers.edit.txt.host" name="host" validation="nonEmpty"/>
                    <sm-ep:txt title="centers.edit.txt.port" name="port" validation="port"/>
                    <sm-ep:txt title="centers.edit.txt.althost" name="altHost"/>
                    <sm-ep:txt title="centers.edit.txt.altport" name="altPort" validation="port"/>
                    <sm-ep:txt title="centers.edit.txt.addressrange" name="addressRange"/>
                    <sm-ep:list title="centers.edit.list.mode" name="mode" values="1,2,3" valueTitles="TX,RX,TRX"/>
                    <sm-ep:txt title="centers.edit.txt.timeout" name="timeout" maxlength="6" validation="unsigned"/>
                    <sm-ep:check title="centers.edit.check.enabled.title" head="centers.edit.check.enabled.head" name="enabled"/>
                    <sm-ep:txtEmpty title="sp.edit.txt.inQueueLimit" name="inQueueLimit" maxlength="6" validation="unsignedOrEmpty"/>
                    <sm-ep:txtEmpty title="sp.edit.txt.outQueueLimit" name="outQueueLimit" maxlength="6" validation="unsignedOrEmpty"/>
                    <sm-ep:txtEmpty title="sp.edit.txt.maxSmsPerSec" name="maxSmsPerSec" maxlength="6" validation="unsignedOrEmpty"/>
                    <c:choose>
                        <c:when test="${!param.add}">
                            <sm-ep:txt title="centers.edit.txt.uid" name="uid" readonly="true"/>
                        </c:when>
                        <c:otherwise></c:otherwise>
                    </c:choose>
                </sm-ep:properties>
            </c:when>
            <c:when test="${bean.transportId == 2}">
                <sm-ep:properties title="centers.edit.properties.http_info">
                    <br>
                    Place your content here
                </sm-ep:properties>
            </c:when>
            <c:when test="${bean.transportId == 3}">
                <sm-ep:properties title="centers.edit.properties.mms_info">
                    <br>
                    Place your content here
                </sm-ep:properties>
            </c:when>
        </c:choose>
    </jsp:body>
</sm:page>