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

		public static class services
		{
			private static final String B = SMSCErrors.error.B + "services.";
			public static final String couldntStartInternalService = B + "couldntStartInternalService";
			public static final String unknownAction = B + "unknownAction";
			public static final String couldntGetServiceInfo = B + "couldntGetServiceInfo";
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
