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
  }


  protected Constants(String id)
  {
    this.id = id;
  }

  public String getId()
  {
    return id;
  }
}
