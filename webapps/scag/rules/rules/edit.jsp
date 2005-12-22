<%@ include file="/WEB-INF/inc/header.jspf" %>

<sm:page>
    <jsp:attribute name="title">Create new rule
    </jsp:attribute>
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <%--<sm-pm:item name="mbNext" value="Next>>>" title="Create rule"/>--%>
            <sm-pm:item name="mbSave" value="Save" title="Save rule"/>
            <sm-pm:item name="mbCancel" value="Cancel" title="Cancel add a new rule" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="">
            <sm-ep:txt title="Name:" name="name" validation="ruleName"/>
            <sm-ep:list title="Transport" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
            <c:choose>
                <c:when test="${bean.administrator}">
                    <sm-ep:list title="provider" name="providerId"
                                values="${fn:join(bean.providerIds, ',')}"
                                valueTitles="${fn:join(bean.providerNames, ',')}"/>
                </c:when>
                <c:otherwise>
                    <sm-ep:const title="provider" name="providerName" value="${bean.providerName}"/>
                </c:otherwise>
            </c:choose>
        </sm-ep:properties>
    </jsp:body>
</sm:page>

