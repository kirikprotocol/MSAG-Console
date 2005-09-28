<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Rules">
    <jsp:attribute name="menu">


        <sm-pm:menu>

            <sm-pm:item name="mbAdd" value="Add" title="Add new rule" onclick="clearForm()"/>
            <sm-pm:item name="mbDelete" value="Delete" title="Delete rule" isCheckSensible="true"/>
            <sm-pm:space/>
        </sm-pm:menu>

    </jsp:attribute>

    <jsp:body>

        <sm:table columns="checkbox,name,providerName,notes,transport" names="c,name,provider,notes,transport"
                  widths="1,24,25,25,25" edit="name" goal="jedit"/>
    </jsp:body>
</sm:page>