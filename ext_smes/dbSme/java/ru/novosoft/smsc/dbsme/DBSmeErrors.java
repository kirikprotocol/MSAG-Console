package ru.novosoft.smsc.dbsme;

/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:51:19 PM
 */
public class DBSmeErrors {
	private static String B = "";

	public static class error {
		private static final String B = DBSmeErrors.B + "error.dbSme.";
		public static final String couldntGetConfig = "Couldn't get config";//B + "couldntGetConfig";
		public static final String couldntFindConfig = "Couldn't find original config";//B + "couldntFindConfig";
		public static final String couldntSaveTempConfig = "Couldn't save config";//B + "couldntSaveTempConfig";
		public static final String couldntStartDbSme = "Couldn't start DB SME";//B + "couldntStartDbSme";
		public static final String couldntStopDbSme = "Couldn't stop DB SME";//B + "couldntStopDbSme";
		public static final String couldntRemoveOldConfig = "Couldn't remove original config";//B + "couldntRemoveOldConfig";
		public static final String couldntApplyNewConfig = "Couldn't apply new config";//B + "couldntApplyNewConfig";
		public static final String couldntGetOriginalConfig = "Couldn't get original config";//B + "couldntGetOriginalConfig";

		public static class provider {
			private static final String B = DBSmeErrors.error.B + "provider.";
			public static final String providerNotSpecified = "Provider not specified";//B + "providerNotSpecified";
			public static final String providerAlreadyExists = "Provider already exists";//B + "providerAlreadyExists";
		}

		public static class job {
			private static final String B = DBSmeErrors.error.B + "job.";
			public static final String unknownType = "Unknown job type";//B + "unknownType";
			public static final String couldntGetJobProperties = "Couldn't get job properties";//B + "couldntGetJobProperties";
			public static final String jobNotFound = "Job not found";//B + "jobNotFound";
			public static final String jobAlreadyExists = "Job already exists";//B + "jobAlreadyExists";
		}
	}
}
