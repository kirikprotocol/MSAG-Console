package ru.novosoft.smsc.admin.sme;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.resource.ResourceSettings;
import ru.novosoft.smsc.admin.resource.Resources;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
@Aspect
public class ResourceDependencyAspect {

  @Pointcut("call (void SmeManager.addSme(String, Sme))")
  public void addSme() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.resource.ResourceManager.addResourceSettings(*,*))")
  public void addResourceSettings() {
  }

  @Pointcut("call (boolean ru.novosoft.smsc.admin.resource.ResourceManager.removeResourceSettings(*))")
  public void removeResource() {}

  private boolean checkReceiptScheme(Resources receiptScheme) throws SmeException {
    if (receiptScheme == null)
      return false;
    if (receiptScheme.getString("deletedFormat") == null)
      return false;
    if (receiptScheme.getString("deliveredFormat") == null)
      return false;
    if (receiptScheme.getString("failedFormat") == null)
      return false;
    if (receiptScheme.getString("notifyFormat") == null)
      return false;
    return true;
  }

  @Before("target(smeManager) && args(*, sme) && addSme()")
  public void beforeAddSme(SmeManager smeManager, Sme sme) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(smeManager);
    if (ctx == null)
      return;

    ResourceManager resourceManager = ctx.getResourceManager();

    for (String locale : resourceManager.getLocales()) {
      ResourceSettings s = resourceManager.getResourceSettings(locale);

      Resources receipt = s.getResources().getChild("receipt");
      if (receipt == null)
        throw new SmeException("receipt_scheme_not_found", locale);
      Resources receiptScheme = receipt.getChild(sme.getReceiptSchemeName());
      if (!checkReceiptScheme(receiptScheme))
        throw new SmeException("receipt_scheme_not_found", locale);
    }
  }

  @Before("target(resourceManager) && args(*, resourceSettings) && addResourceSettings()")
  public void beforeRemoveResource(ResourceManager resourceManager, ResourceSettings resourceSettings) throws AdminException {

    AdminContext ctx = AdminContextLocator.getContext(resourceManager);
    if (ctx == null)
      return;

    Resources receipt = resourceSettings.getResources().getChild("receipt");

    SmeManager smeManager = ctx.getSmeManager();
    for (Map.Entry<String, Sme> e : smeManager.smes().entrySet()) {
      String smeId = e.getKey();
      Sme sme = e.getValue();

      if (receipt == null)
        throw new SmeException("receipt_scheme_used", smeId);
      Resources receiptScheme = receipt.getChild(sme.getReceiptSchemeName());
      if (!checkReceiptScheme(receiptScheme))
        throw new SmeException("receipt_scheme_used", smeId);
    }

  }

  @Before("target(resourceManager) && args(locale) && removeResource()")
  public void beforeRemoveResource(ResourceManager resourceManager, String locale) throws AdminException {
    System.out.println("!!!->");
  }

}
