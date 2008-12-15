package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsx.engine.service.group.commands.*;

/**
 * User: artem
 * Date: 15.07.2008
 */

public interface GroupService extends Service, GroupSendCmd.Receiver,
                                GroupSendStatusCmd.Receiver,
                                GroupReplyCmd.Receiver,
                                GroupDeliveryReportCmd.Receiver,
                                GroupAddCmd.Receiver,
                                GroupAlterCmd.Receiver,
                                GroupRemoveCmd.Receiver,
                                GroupRenameCmd.Receiver,
                                GroupCopyCmd.Receiver,  
                                GroupAddMemberCmd.Receiver,
                                GroupRemoveMemberCmd.Receiver,
                                GroupInfoCmd.Receiver,
                                GroupListCmd.Receiver,
                                GroupEditAlterProfileCmd.Receiver,
                                GroupEditGetProfileCmd.Receiver,
                                GroupAddSubmitterCmd.Receiver,
                                GroupRemoveSubmitterCmd.Receiver,
                                PrincipalAddCmd.Receiver,
                                PrincipalRemoveCmd.Receiver,
                                PrincipalListCmd.Receiver,
                                PrincipalGetCmd.Receiver,
                                PrincipalAlterCmd.Receiver {
    
}
