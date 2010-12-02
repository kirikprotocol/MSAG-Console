package mobi.eyeline.informer.admin.delivery.stat;

/**
 * @author Artem Snopkov
 */
public interface UserStatVisitor {

  boolean visit(UserStatRecord rec, int total, int current);
}
