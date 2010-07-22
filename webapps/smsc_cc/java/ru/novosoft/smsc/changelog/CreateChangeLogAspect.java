package ru.novosoft.smsc.changelog;

import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;

/**
 * Аспект, требующий, чтобы при создании каждого инстанца AdminContext, создавался соответствующий инстанц ChangeLog 
 * @author Artem Snopkov
 */
@Aspect
public class CreateChangeLogAspect {

  @Pointcut("initialization (ru.novosoft.smsc.admin.AdminContext.new()) ||" +
            "initialization (ru.novosoft.smsc.admin.AdminContext.new(File, File))")
  public void adminContextInit() {}

  @AfterReturning("target(ctx) && adminContextInit()")
  public void afterAdminContextInit(AdminContext ctx) {
    ChangeLog.addInstance(ctx);
  }

}
