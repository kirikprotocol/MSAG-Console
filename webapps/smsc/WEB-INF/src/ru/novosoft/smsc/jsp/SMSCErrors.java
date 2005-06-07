package ru.novosoft.smsc.jsp;

/*
 * Created by igork
 * Date: 25.10.2002
 * Time: 17:27:47
 */


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
	public static final String notYetImplemented = B + "notYetImplemented";
	public static final String CouldntGetSMSCStatus = B + "CouldntGetSMSCStatus";
	public static final String CouldntGetBundleKey = B + "CoulndtGetBundleKey";


	public static class sql
	{
		private static final String B = SMSCErrors.error.B + "sql.";
		public static final String exception = B + "exception";
		public static final String coulndtCloseDBConnection = B + "couldntCloseDBConnection";
	}
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
		public static final String NotAllParametersDefined = B + "NotAllParametersDefined";
		public static final String ServiceIdNotDefined = B + "ServiceIdNotDefined";
		public static final String ServiceIdTooLong = B + "ServiceIdTooLong";
		public static final String ServiceNotFound = B + "ServiceNotFound";
		public static final String fileNotAttached = B + "fileNotAttached";
		public static final String serviceDistributiveNotAttached = B + "serviceDistributiveNotAttached";
		public static final String serviceAlreadyExists = B + "serviceAlreadyExists";
		public static final String distributiveFileMustBeZipCompressed = B + "distributiveFileMustBeZipCompressed";
		public static final String couldntReceiveFile = B + "couldntReceiveFile";
		public static final String hostNotSelected = B + "hostNotSelected";
		public static final String incorrectPortValue = B + "incorrectPortValue";
		public static final String couldntSaveWebXml = B + "couldntSaveWebXml";
		public static final String somethingGlobalIsChanged = B + "somethingGlobalIsChanged";
		public static final String notAuthorizedForDeletingService = B + "notAuthorizedForDeletingService";
		public static final String serviceManagerIsNull = B + "serviceManagerIsNull";
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
      public static final String servicesFolderNotSpecified = B + "servicesFolderNotSpecified";
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
      public static final String invalidLocale = B + "invalidLocale";
      public static final String couldntGetRegisteredLocales = B + "couldntGetRegisteredLocales";
      public static final String couldntDelete = B + "couldntDelete";
      public static final String couldntUpdate = B + "couldntUpdate";
    }


    public static class aliases
    {
      private static final String B = error.B + "aliases.";
      public static final String alreadyExists = B + "alreadyExists";
      public static final String alreadyExistsAddress = B + "alreadyExistsAddress";
      public static final String cantAdd = B + "cantAdd";
      public static final String cantEdit = B + "cantEdit";
      public static final String invalidAlias = B + "invalidAlias";
      public static final String invalidAddress = B + "invalidAddress";
      public static final String HideWithQuestion = B + "invalidAliasHideWithQuestion";
      public static final String QuestionCountsNotMathes = B + "QuestionCountsNotMathes";
      public static final String alreadyExistsAlias = B + "alreadyExistsAlias";
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
		public static final String incorrectMask = B + "incorrectMask";
		public static final String incorrectFilterMask = B + "incorrectFilterMask";
		public static final String subjNotFound = B + "subjNotFound";
		public static final String defaultSmeNotFound = B + "defaultSmeNotFound";
	}


    public static class routes
    {
      private static final String B = error.B + "routes.";
      public static final String alreadyExists = B + "alreadyExists";
      public static final String cantAdd = B + "cantAdd";
      public static final String cantEdit = B + "cantEdit";
      public static final String cantSave = B + "cantSave";
      public static final String cantLoad = B + "cantLoad";
      public static final String cantRestore = B + "cantRestore";
      public static final String LoadAndCheckFailed = B + "loadAndCheckFailed";
      public static final String TraceRouteFailed = B + "TraceRouteFailed";
      public static final String nameNotSpecified = B + "nameNotSpecified";
      public static final String invalidPriority = B + "invalidPriority";
      public static final String sourcesIsEmpty = B + "sourcesIsEmpty";
      public static final String destinationsIsEmpty = B + "destinationsIsEmpty";
      public static final String invalidDestinationMask = B + "invalidDestinationMask";
      public static final String invalidSourceMask = B + "invalidSourceMask";
      public static final String srcAddressIsAlias = B + "srcAddressIsAlias";
      public static final String dstAddressIsAlias = B + "dstAddressIsAlias";
      public static final String CantUpdateFilter = B + "CantUpdateFilter";

    }


    public static class users
    {
      private static final String B = error.B + "users.";
      public static final String userNotFound = B + "userNotFound";
      public static final String userAlreadyExists = B + "userAlreadyExists";
      public static final String couldntApply = B + "couldntApply";
      public static final String passwordNotDefined = B + "passwordNotDefined";
      public static final String passwordNotConfirmed = B + "passwordNotConfirmed";
      public static final String loginNotDefined = B + "loginNotDefined";
    }

    public static class providers
    {
      private static final String B = error.B + "providers.";
      public static final String providerNotFound = B + "providerNotFound";
      public static final String providerAlreadyExists = B + "providerAlreadyExists";
      public static final String couldntApply = B + "couldntApply";
      public static final String nameNotDefined = B + "nameNotDefined";
      public static final String provider_not_found = B + "Provider not found";

    }

    public static class categories
    {
      private static final String B = error.B + "categories.";
      public static final String categoryNotFound = B + "categoryNotFound";
      public static final String categoryAlreadyExists = B + "categoryAlreadyExists";
      public static final String couldntApply = B + "couldntApply";
      public static final String nameNotDefined = B + "nameNotDefined";
      public static final String category_not_found = B + "Category not found";

    }

	public static class smsc
	{
		private static final String B = error.B + "smsc.";
		public static final String contextInitFailed = B + "contextInitFailed";
		public static final String daemonNotFound = B + "daemonNotFound";
		public static final String couldntRefreshStatus = B + "couldntRefreshStatus";
		public static final String couldntStart = B + "couldntStart";
		public static final String couldntStop = B + "couldntStop";
		public static final String couldntApply = B + "couldntApply";
		public static final String couldntSave = B + "couldntSave";
		public static final String couldntRefreshComponents = B + "couldntRefreshComponents";
		public static final String couldntGetConfig = B + "couldntGetConfig";
		public static final String couldntGetScheduleConfig = B + "couldntGetScheduleConfig";
		public static final String invalidParameter = B + "invalidParameter";
		public static final String invalidIntParameter = B + "invalidIntParameter";
		public static final String invalidBoolParameter = B + "invalidBoolParameter";
		public static final String couldntGetLogCats = B + "couldntGetLogCats";
		public static final String couldntSetLogCats = B + "couldntSetLogCats";


      public static class reshedule
      {
        private static final String B = smsc.B + "reshedule.";
        public static final String couldntGetErrorCodes = B + "couldntGetErrorCodes";
        public static final String couldntSaveConfig = B + "couldntSaveConfig";
		public static final String couldntResetConfig = B + "couldntResetConfig";
        public static final String couldntGetReshedules = B + "couldntGetReshedules";
        public static final String resheduleAlreadyPresent = B + "resheduleAlreadyPresent";
        public static final String couldntGetResheduleInstance = B + "couldntGetResheduleInstance";
      }
    }


    public static class commutators
    {
      private static final String B = error.B + "smsc.";
      public static final String smscServerError = B + "smscServerError";
    }


    public static class localeResources
    {
      private static final String B = error.B + "localeResources.";
      public static final String incorrectLocaleName = B + "incorrectLocaleName";
      public static final String localeResourcesFileNotFound = B + "localeResourcesFileNotFound";
      public static final String fileNotAttached = B + "fileNotAttached";
      public static final String wrongFileType = B + "wrongFileType";
      public static final String wrongFileName = B + "wrongFileName";
      public static final String alreadyExists = B + "alreadyExists";
      public static final String couldntReceiveFile = B + "couldntReceiveFile";
      public static final String couldntCreateFile = B + "couldntCreateFile";
      public static final String couldntStoreFile = B + "couldntStoreFile";
      public static final String couldntApplyResources = B + "couldntApplyResources";
    }


	public static class dl
	{
		private static final String B = error.B + "dl.";
		public static final String CouldntListDistributionLists = B + "CouldntListDistributionLists";
		public static final String couldntdelete = B + "couldntdelete";
		public static final String unknownDL = B + "unknownDL";
		public static final String couldntadd = B + "couldntadd";
		public static final String couldntaddAlreadyexists = B + "couldntaddAlreadyexists";
		public static final String couldntListLists = B + "couldntListLists";
		public static final String dlNotDefined = B + "dlNotDefined";
		public static final String wildcardsNotAllowedInAddress = B + "wildcardsNotAllowedInAddress";
		public static final String couldntSave = B + "couldntSave";
		public static final String couldntGetSubmitters = B + "couldntGetSubmitters";
		public static final String couldntGetMembers = B + "couldntGetMembers";
		public static final String invalidAddress = B + "invalidAddress";
		public static final String noSubmittersEntered = B + "noSubmittersEntered";
		public static final String noMembersEntered = B + "noMembersEntered";
		public static final String tooMuchMembers = B + "tooMuchMembers";
		public static final String ownerNotSpecified = B + "ownerNotSpecified";
		public static final String unknownOwner = B + "unknownOwner";
		public static final String couldntFindOwner = B + "couldntFindOwner";
		public static final String couldntAlterDL = B + "couldntAlterDL";
		public static final String exceedMaxMembersValue = B + "exceedMaxMembersValue";
		public static final String patternIsInvalid = B + "patternIsInvalid";
		public static final String principalNotFound = B + "principalNotFound";
		public static final String cannotLoadPrincipal = B + "cannotLoadPrincipal";
		public static final String principalAlreadyExist = B + "principalAlreadyExist";
		public static final String couldntCreatePrincipal = B + "couldntCreatePrincipal";
		public static final String couldntUpdatePrincipal = B + "couldntUpdatePrincipal";
		public static final String couldntDeletePrincipal = B + "couldntDeletePrincipal";
		public static final String maxListsCountTooSmall = B + "maxListsCountTooSmall";
		public static final String maxMembersCountTooSmall = B + "maxMembersCountTooSmall";
		public static final String maxListsIncorrectValue = B + "maxListsIncorrectValue";
		public static final String maxElementsIncorrectValue = B + "maxElementsIncorrectValue";
    }


	public static class smsview
	{
		private static final String B = error.B + "smsview.";
		public static final String InitFailed = B + "InitFailed";
		public static final String QueryFailed = B + "QueryFailed";
		public static final String DeleteFailed = B + "DeleteFailed";
		public static final String AccessDeniedToOperative = B + "AccessDeniedToOperative";
		public static final String AccessDeniedToArchive = B + "AccessDeniedToArchive";
	}


    public static class acl
    {
      private static final String B = error.B + "acl.";
      public static final String COULDNT_GET_ACL_NAMES = B + "couldntGetAclNames";
      public static final String COULDNT_LOOKUP_ACL = B + "couldntLookupAcl";
      public static final String COULDNT_DELETE_ACL = B + "couldntDeleteAcl";
      public static final String COULDNT_GET_ACL_INFO = B + "couldntGetAclInfo";
      public static final String COULDNT_UPDATE_ACL_INFO = B + "couldntUpdateAclInfo";
      public static final String COULDNT_CREATE_ACL = B + "couldntCreateAcl";
      public static final String COULDNT_LOOKUP_ADDRESSES = B + "couldntLookupAddresses";
      public static final String COULDNT_REMOVE_ADD_ADDRESSES = B + "couldntRemoveAddAddresses";
    }

	public static class smsstat
	{
		private static final String B = error.B + "smsstat.";
		public static final String QueryFailed = B + "QueryFailed";
		public static final String StatError = B + "StatError";
    }
     public static class smsexport
    {
      private static final String B = error.B + "smsexport.";
      public static final String ExportFailed = B + "ExportFailed";
    }
  }


  public static class warning
  {
    private static final String B = SMSCErrors.B + "warning.";
    public static final String SMSCNotRunning = B + "SMSCNotRunning";


    public static class hosts
    {
      private static final String B = warning.B + "hosts.";

      public static final String listFailed = B + "listFailed";
      public static final String noServicesSelected = B + "noServicesSelected";
    }
  }
}
