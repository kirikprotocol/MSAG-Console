/*
 * Created by igork
 * Date: 25.10.2002
 * Time: 17:27:47
 */
package ru.novosoft.smsc.jsp;

public class SMSCErrors
{
	private static String B = "";

	public static class error
	{
		private static final String B = SMSCErrors.B + "error.";
		public static final String failed = B + "failed";
		public static final String unknown = B + "unknown";
		public static final String system = B + "system";
		public static final String errorListNotInitialized = B + "errorListNotInitialized";
		public static final String appContextNotInitialized = B + "appContextNotInitialized";
		public static final String serviceManagerNotInitialized = B + "serviceManagerNotInitialized";
		public static final String daemonManagerNotInitialized = B + "daemonManagerNotInitialized";
		public static final String couldntApplyChanges = B + "couldntApplyChanges";

		public static class services
		{
			private static final String B = SMSCErrors.error.B + "services.";
			public static final String couldntStartInternalService = B + "couldntStartInternalService";
			public static final String unknownAction = B + "unknownAction";
			public static final String couldntGetServiceInfo = B + "couldntGetServiceInfo";
			public static final String coudntDeleteService = B + "coudntDeleteService";
			public static final String coudntAddService = B + "coudntAddService";
			public static final String invalidPriority = B + "invalidPriority";
			public static final String alreadyExists = B + "alreadyExists";
		}

		public static class hosts
		{
			private static final String B = error.B + "hosts.";
			public static final String daemonNotFound = B + "daemonNotFound";
			public static final String serviceNotFound = B + "serviceNotFound";
			public static final String couldntStartService = B + "couldntStartService";
			public static final String couldntStopService = B + "couldntStopService";
			public static final String hostNotSpecified = B + "hostNotSpecified";
			public static final String hostsNotSpecified = B + "hostsNotSpecified";
			public static final String hostAlreadyExist = B + "hostAlreadyExist";
			public static final String portNotSpecifiedOrIncorrect = B + "portNotSpecifiedOrIncorrect";
			public static final String couldntAddHost = B + "couldntAddHost";
			public static final String couldntRemoveHost = B + "couldntRemoveHost";
		}

		public static class profiles
		{
			private static final String B = error.B + "profiles.";
			public static final String queryError = B + "queryError";
			public static final String identicalToDefault = B + "identicalToDefault";
			public static final String couldntAdd = B + "couldntAdd";
			public static final String profileNotSpecified = B + "profileNotSpecified";
			public static final String couldntLookup = B + "couldntLookup";
			public static final String invalidMask = B + "invalidMask";
			public static final String smscNotConnected = B + "smscNotConnected";
		}

		public static class aliases
		{
			private static final String B = error.B + "aliases.";
			public static final String alreadyExists = B + "alreadyExists";
			public static final String cantAdd = B + "cantAdd";
			public static final String cantEdit = B + "cantEdit";
			public static final String invalidAlias = B + "invalidAlias";
			public static final String invalidAddress = B + "invalidAddress";
		}

		public static class subjects
		{
			private static final String B = error.B + "subjects.";
			public static final String alreadyExists = B + "alreadyExists";
			public static final String cantAdd = B + "cantAdd";
			public static final String cantDelete = B + "cantDelete";
			public static final String cantEdit = B + "cantEdit";
			public static final String subjNotSpecified = B + "subjNotSpecified";
			public static final String masksNotDefined = B + "masksNotDefined";
		}

		public static class routes
		{
			private static final String B = error.B + "routes.";
			public static final String alreadyExists = B + "alreadyExists";
			public static final String cantAdd = B + "cantAdd";
			public static final String cantEdit = B + "cantEdit";
			public static final String nameNotSpecified = B + "nameNotSpecified";
			public static final String invalidPriority = B + "invalidPriority";
		}

		public static class users
		{
			private static final String B = error.B + "users.";
			public static final String userNotFound = B + "userNotFound";
			public static final String userAlreadyExists = B + "userAlreadyExists";
			public static final String couldntApply = B + "couldntApply";
			public static final String passwordNotDefined = B + "passwordNotDefined";
			public static final String passwordNotConfirmed = B + "passwordNotConfirmed";
		}

		public static class smsc
		{
			private static final String B = error.B + "smsc.";
			public static final String daemonNotFound = B + "daemonNotFound";
			public static final String couldntRefreshStatus = B + "couldntRefreshStatus";
			public static final String couldntStart = B + "couldntStart";
			public static final String couldntStop = B + "couldntStop";
			public static final String couldntRefreshComponents = B + "couldntRefreshComponents";
			public static final String couldntGetConfig = B + "couldntGetConfig";
			public static final String invalidParameter = B + "invalidParameter";
			public static final String invalidIntParameter = B + "invalidIntParameter";
			public static final String invalidBoolParameter = B + "invalidBoolParameter";
		}
	}

	public static class warning
	{
		private static final String B = SMSCErrors.B + "warning.";

		public static class services
		{
			private static final String B = SMSCErrors.warning.B + "services.";
		}

		public static class hosts
		{
			private static final String B = warning.B + "hosts.";

			public static final String listFailed = B + "listFailed";
			public static final String noServicesSelected = B + "noServicesSelected";
		}
	}
}
