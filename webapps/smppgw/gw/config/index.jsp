<%@include file="/WEB-INF/inc/header.jspf"%>
<script src="tree.js"></script>
<sm:page title="Edit SMPP GW configuration">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save user info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel user editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
      <sm-et:section title="Data Source" name="DataSource">
        <sm-et:properties>
          <sm-et:txt title="connections" name="connections" validation="positive"/>
          <sm-et:txt title="DB instance" name="dbInstance" validation="nonEmpty"/>
          <sm-et:txt title="DB user name" name="dbUserName" validation="nonEmpty"/>
          <sm-et:txt title="DB user password" name="dbUserPassword"/>
          <sm-et:txt title="type" name="type" validation="nonEmpty"/>
        </sm-et:properties>
      </sm-et:section>

      <sm-et:section title="Startup Loader" name="StartupLoader">
        <sm-et:section title="Data Source Drivers" name="DataSourceDrivers">
          <sm-et:section title="OCI Data Source Driver" name="OCIDataSourceDriver">
            <sm-et:properties>
              <sm-et:txt name="loadup" validation="nonEmpty"/>
              <sm-et:txt name="type" validation="nonEmpty"/>
            </sm-et:properties>
          </sm-et:section>
        </sm-et:section>
      </sm-et:section>


      <sm-et:section title="Administration" name="admin">
        <sm-et:properties>
          <sm-et:txt name="host" type="string"/>
          <sm-et:txt name="port" type="int" validation="port"/>
        </sm-et:properties>
      </sm-et:section>

      <sm-et:section title="Core" name="core">
        <sm-et:properties>
          <sm-et:txt name="default_locale"/>
          <sm-et:txt name="eventQueueLimit"/>
          <sm-et:txt name="locales"/>
          <sm-et:txt name="state_machines_count"/>
        </sm-et:properties>

        <sm-et:section title="Performance" name="performance">
          <sm-et:properties>
            <sm-et:txt name="host"/>
            <sm-et:txt name="port" validation="port"/>
          </sm-et:properties>
        </sm-et:section>
      </sm-et:section>

      <sm-et:section title="Logger" name="logger">
        <sm-et:properties>
          <sm-et:txt title="init file" name="initFile"/>
        </sm-et:properties>
      </sm-et:section>

      <sm-et:section title="SMPP" name="smpp">
        <sm-et:properties>
          <sm-et:txt name="host"/>
          <sm-et:txt name="inactivityTime"/>
          <sm-et:txt name="inactivityTimeOut"/>
          <sm-et:txt name="port"/>
          <sm-et:txt name="readTimeout"/>
        </sm-et:properties>
      </sm-et:section>

  </jsp:body>
</sm:page>