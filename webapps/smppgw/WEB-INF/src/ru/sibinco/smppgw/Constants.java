/**
 * Created by igork
 * Date: 12.03.2004
 * Time: 13:28:42
 */
package ru.sibinco.smppgw;


public class Constants
{
  private final String id;


  public interface errors
  {
    public interface status
    {
      public static final Constants COULDNT_APPLY_CONFIG = new Constants("error.status.couldnt_apply_config");
      public static final Constants COULDNT_SAVE_CONFIG = new Constants("error.status.couldnt_save_config");
      public static final Constants COULDNT_APPLY_ROUTES = new Constants("error.status.couldnt_apply_routes");
      public static final Constants COULDNT_START_GATEWAY = new Constants("error.status.couldnt_start_gateway");
      public static final Constants COULDNT_STOP_GATEWAY = new Constants("error.status.couldnt_stop_gateway");
      public static final Constants COULDNT_REFRESH_SERVICES = new Constants("error.status.couldnt_refresh_services");
      public static final Constants COULDNT_GET_DAEMON = new Constants("error.status.couldnt_get_daemon");
      public static final Constants COULDNT_APPLY_USERS = new Constants("error.status.couldnt_apply_users");
      public static final Constants COULDNT_APPLY_SMSCS = new Constants("error.status.couldnt_apply_smscs");
      public static final Constants COULDNT_APPLY_PROVIDERS = new Constants("error.status.couldnt_apply_providers");
      public static final Constants COULDNT_APPLY_BILLING = new Constants("error.status.couldnt_apply_billing");
    }


    public interface config
    {
      public static final Constants INVALID_INTEGER = new Constants("error.config.invalid_integer");
    }


    public interface routing
    {
      public interface subjects
      {
        public static final Constants SUBJECT_ALREADY_EXISTS = new Constants("error.routing.subjects.subject_already_exists");
        public static final Constants COULD_NOT_CREATE = new Constants("error.routing.subjects.could_not_create");
        public static final Constants DEFAULT_SME_NOT_FOUND = new Constants("error.routing.subjects.default_sme_not_found");
        public static final Constants COULD_NOT_SET_MASKS = new Constants("error.routing.subjects.could_not_set_masks");
      }


      public interface routes
      {
        public static final Constants ROUTE_ALREADY_EXISTS = new Constants("error.routing.routes.route_already_exists");
        public static final Constants DEST_SUBJ_NOT_FOUND = new Constants("error.routing.routes.dest_subj_not_found");
        public static final Constants SME_NOT_FOUND = new Constants("error.routing.routes.sme_not_found");
        public static final Constants COULD_NOT_CREATE_DESTINATION = new Constants("error.routing.routes.could_not_create_destination");
        public static final Constants COULD_NOT_CREATE_MASK = new Constants("error.routing.routes.could_not_create_mask");
        public static final Constants COULD_NOT_CREATE_DESTINATION_MASK = new Constants("error.routing.routes.could_not_create_destination_mask");
      }
    }


    public interface users
    {
      public static final Constants LOGIN_NOT_SPECIFIED = new Constants("error.user.login_not_specified");
      public static final Constants USER_NOT_FOUND = new Constants("error.user.user_not_found");
      public static final Constants PASSWORD_NOT_CONFIRM = new Constants("error.user.password_not_confirm");
      public static final Constants USER_ALREADY_EXISTS = new Constants("error.user.user_already_exists");
      public static final Constants PASSWORD_NOT_SPECIFIED = new Constants("error.user.password_not_specified");
    }


    public interface sme
    {
      public static final Constants SME_NOT_FOUND = new Constants("error.sme.sme_not_found");
      public static final Constants SME_ID_NOT_SPECIFIED = new Constants("error.sme.sme_id_not_specified");
      public static final Constants SME_ALREADY_EXISTS = new Constants("error.sme.sme_already_exists");
      public static final Constants COULDNT_APPLY = new Constants("error.sme.couldnt_apply");
      public static final Constants COULDNT_DELETE = new Constants("error.sme.couldnt_delete");
    }


    public interface providers
    {
      public static final Constants PROVIDER_NOT_FOUND = new Constants("error.providers.provider_not_found");
      public static final Constants COULDNT_DELETE_PROVIDER = new Constants("error.providers.couldnt_delete_provider");
    }


    public interface smscs
    {
      public static final Constants SMSC_NOT_FOUND = new Constants("error.smscs.smsc_not_found");
    }


    public interface stat
    {
      public static final Constants GET_STATISTICS_FAILED = new Constants("error.stat.get_statistics_failed");
    }

  }


  protected Constants(final String id)
  {
    this.id = id;
  }

  public String getId()
  {
    return id;
  }
}
