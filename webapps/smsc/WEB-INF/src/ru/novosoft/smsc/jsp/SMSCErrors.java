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

		public static class service
		{
			private static final String B = SMSCErrors.error.B + "service.";
			public static final String failed = B + "failed";
			public static final String unknown = B + "unknown";
			public static final String system = B + "system";
			public static final String unknownAction = B + "unknownAction";

			public static class hosts
			{
				private static final String B = SMSCErrors.error.service.B + "hosts.";
				public static final String daemonNotFound = B + "daemonNotFound";
				public static final String serviceNotFound = B + "serviceNotFound";
				public static final String couldntStartService = B + "couldntStartService";
				public static final String couldntStopService = B + "couldntStopService";
			}
		}
	}

	public static class warning
	{
		private static final String B = SMSCErrors.B + "warning.";

		public static class service
		{
			private static final String B = SMSCErrors.warning.B + "service.";

			public static class hosts
			{
				private static final String B = SMSCErrors.warning.service.B + "hosts.";

				public static final String listFailed = B + "listFailed";
				public static final String noServicesSelected = B + "noServicesSelected";
			}
		}
	}
}
