header {
package ru.novosoft.smsc.admin.console.parser;

/*
 * Parser code generated by ANTLR
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.console.commands.*;
import ru.novosoft.smsc.admin.console.commands.acl.*;
import ru.novosoft.smsc.admin.console.commands.alias.*;
import ru.novosoft.smsc.admin.console.commands.dl.*;
import ru.novosoft.smsc.admin.console.commands.profile.*;
import ru.novosoft.smsc.admin.console.commands.route.*;
import ru.novosoft.smsc.admin.console.commands.sme.*;
import ru.novosoft.smsc.admin.console.commands.misc.*;
import ru.novosoft.smsc.admin.console.commands.apply.*;
import ru.novosoft.smsc.admin.console.commands.closedgroup.*;
import ru.novosoft.smsc.admin.console.commands.emailsme.*;

}

/*
{
import java.io.*;

class Main {
	public static void main(String[] args) {
		try {
			CommandLexer lexer = new CommandLexer(new DataInputStream(System.in));
			CommandParser parser = new CommandParser(lexer);
			CommandContext ctx = new CommandContext(null);
			parser.parse(ctx);
		} catch(Exception e) {
			System.err.println("exception: "+e);
			e.printStackTrace(System.err);   // so we can get stack trace
		}
	}
}
}
*/

class CommandParser extends Parser;
options {
	importVocab=CommandLexer; // use vocab generated by lexer
	defaultErrorHandler=false;
}

/* ----------------------- Top level parser ----------------------- */
parse returns [Command cmd] {
    cmd = null;
}
	: ACT_ADD	cmd = add
	| ACT_DELETE 	cmd = del
	| ACT_ALTER 	cmd = alt
	| ACT_LIST	cmd = lst
	| ACT_VIEW	cmd = view
	| ACT_SHOW	cmd = show
	| ACT_GRANT	cmd = grant
	| ACT_REVOKE	cmd = revoke
	| ACT_CHECK	cmd = check
	| ACT_EXPORT	cmd = export
	| ACT_APPLY	cmd = apply
	;

/* --------------------- Apply action parser ---------------------- */
apply returns [Command cmd] {
    cmd = null;
}
	: TGT_ALL	     { cmd = new ApplyAllCommand();        }
	| TGT_ROUTES	 { cmd = new ApplyRoutesCommand();     }
	| TGT_ALIASES	 { cmd = new ApplyAliasesCommand();    }
	| TGT_PROVIDERS  { cmd = new ApplyProvidersCommand();  } 
	| TGT_CATEGORIES { cmd = new ApplyCategoriesCommand(); }
	;

/* ----------------------- Add action parser ---------------------- */
add returns [Command cmd] {
    cmd = null;
}
	: TGT_ROUTE	cmd = addroute
	| TGT_ALIAS	cmd = addalias
	| TGT_SUBJECT	cmd = addsubject
	| TGT_PROFILE	cmd = addprofile
	| TGT_PRINCIPAL	cmd = addprincipal
	| TGT_ACL	cmd = addacl
	| TGT_DL	cmd = adddl
	| TGT_DLSUB	cmd = adddlsubmitter
	| TGT_DLMEM	cmd = adddlmember
	| TGT_SME	cmd = addsme
	| TGT_PROVIDER	cmd = addprovider
	| TGT_CATEGORY	cmd = addcategory
	| TGT_GROUP     cmd = addgroup
	| TGT_EMAILSME	cmd = addemailsme
	;
	
/* ----------------------- Del action parser ---------------------- */
del returns [Command cmd] {
    cmd = null;
}
	: TGT_ROUTE	cmd = delroute
	| TGT_ALIAS	cmd = delalias
	| TGT_SUBJECT	cmd = delsubject
	| TGT_PROFILE	cmd = delprofile
	| TGT_PRINCIPAL	cmd = delprincipal
	| TGT_ACL	cmd = delacl
	| TGT_DL	cmd = deldl
	| TGT_DLSUB	cmd = deldlsubmitter
	| TGT_DLMEM	cmd = deldlmember
	| TGT_SME	cmd = delsme
	| TGT_PROVIDER	cmd = delprovider
	| TGT_CATEGORY	cmd = delcategory
	| TGT_GROUP     cmd = delgroup
	| TGT_EMAILSME  cmd = delemailsme
	;
/* ----------------------- Alt action parser ---------------------- */
alt returns [Command cmd] {
    cmd = null;
}
	: TGT_ROUTE	cmd = altroute
	| TGT_ALIAS	cmd = altalias
	| TGT_SUBJECT	cmd = altsubject
	| TGT_PROFILE	cmd = altprofile
	| TGT_PRINCIPAL	cmd = altprincipal
	| TGT_ACL	cmd = altacl
	| TGT_DL	cmd = altdl
	| TGT_SME	cmd = altsme
	| TGT_GROUP cmd = altgroup
	;
/* ----------------------- Lst action parser ---------------------- */
lst returns [Command cmd] {
    cmd = null;
}
	: TGT_ROUTE	{ cmd = new RouteListCommand();     }
	| TGT_ALIAS	{ cmd = new AliasListCommand();     }
	| TGT_SUBJECT	{ cmd = new SubjectListCommand();   }
	| TGT_PRINCIPAL	{ cmd = new PrincipalListCommand(); }
	| TGT_ACL	{ cmd = new AclListCommand();       }
	| TGT_DL	{ cmd = new DistributionListListCommand(); }
	| TGT_SME	{ cmd = new SmeListCommand();       }
	| TGT_PROVIDER	{ cmd = new ProviderListCommand();  }
	| TGT_CATEGORY	{ cmd = new CategoryListCommand();  }
	| TGT_GROUP     { cmd = new ClosedGroupListCommand();     }
	;
/* ----------------------- View action parser --------------------- */
view returns [Command cmd] {
    cmd = null;
}
	: TGT_ROUTE	cmd = viewroute
	| TGT_ALIAS	cmd = viewalias
	| TGT_SUBJECT	cmd = viewsubject
	| TGT_PROFILE	cmd = viewprofile
	| TGT_PRINCIPAL	cmd = viewprincipal
	| TGT_ACL	cmd = viewacl
	| TGT_DL	cmd = viewdl
	| TGT_SME	cmd = viewsme
	| TGT_GROUP  cmd = viewgroup
	| TGT_EMAILSME
	(
	  OPT_ID       cmd = viewemailsmebyid
	| cmd = viewemailsmebyaddr
	)
	;
/* ----------------------- Show action parser --------------------- */
show returns [AliasShowCommand cmd] {
    cmd = null;
}
	:	TGT_ALIAS (addr:STR { 
		    cmd = new AliasShowCommand();
		    cmd.setAddress(addr.getText());   
		})
	;
	exception[addr]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Target address for aliases expected");
	}
/* ----------------------- Grant action parser --------------------- */
grant returns[Command cmd] {
    cmd = null;
}
  : ADD_ACCESS ADD_TO TGT_ACL cmd = grantacl
  ;
/* ----------------------- Revoke action parser -------------------- */
revoke returns[Command cmd] {
    cmd = null;
}
  : ADD_ACCESS ADD_TO TGT_ACL cmd = revokeacl
  ;
/* ----------------------- Check action parser --------------------- */
check returns[Command cmd] {
    cmd = null;
}
  : ADD_ACCESS ADD_TO TGT_ACL cmd = checkacl
  ;
/* ----------------------- Export action parser ---------------------- */
export returns [Command cmd] {
    cmd = null;
}
	: TGT_STATS	cmd = exportstats
	| TGT_STATS_E	cmd = exportstats
	| TGT_SMS	cmd = exportsms
	| TGT_SMS_E	cmd = exportsms
	| TGT_ARCHIVE cmd = exportarchive
	;


/* ----------------------- Common names parser ------------------------- */
getnameid[String msg] returns [String out] {
    out = null; 
}	: (qname:QSTR {
	    out = qname.getText().trim();
	    out = out.substring(1,out.length()-1);
	  }) 
	  |(name:STR {
	    out = name.getText();
	  })
	;
	exception
	catch [RecognitionException ex] {
	    throw new RecognitionException(ex.getMessage()+". "+msg+" expected. ");
	}
/* ----------------------- Common number parsers ------------------------ */
getlongid[String msg] returns [long id] {
    id = 0;
} : (num:STR { id = Long.parseLong(num.getText()); })
  ;
exception
catch [RecognitionException ex] {
    throw new RecognitionException(ex.getMessage()+". "+msg+" expected. ");
}
catch [NumberFormatException ex] {
    throw new RecognitionException(ex.getMessage()+". Long value for <"+msg+"> expected. ");
}

getint[String msg] returns [int i] {
    i = 0;
} : (num:STR { i = Integer.parseInt(num.getText()); })
  ;
exception
catch [RecognitionException ex] {
    throw new RecognitionException(ex.getMessage()+". "+msg+" expected. ");
}
catch [NumberFormatException ex] {
    throw new RecognitionException(ex.getMessage()+". Integer value for <"+msg+"> expected. ");
}

/* ----------------------- Common routes parsers ----------------------- */
srcdef[RouteGenCommand cmd] { // Special command required !!!
    RouteSrcDef def = new RouteSrcDef();
}
	: ((OPT_SUBJ {
	    def.setType(RouteSrcDef.TYPE_SUBJECT);
	    def.setSrc(getnameid("Subject name"));
	  }) 
	 |(OPT_MASK addr:STR {
	    def.setType(RouteSrcDef.TYPE_MASK); 
	    def.setSrc(addr.getText());
	  }))
	 ({ cmd.addSrcDef(def); })
	;
	       	
dstdef[RouteGenCommand cmd, boolean needSmeId] { // Special command required !!!
    RouteDstDef def = new RouteDstDef();
}
	: ((OPT_SUBJ {
	    def.setType(RouteDstDef.TYPE_SUBJECT);
	    def.setDst(getnameid("Subject name"));
	  })
	 |(OPT_MASK addr:STR {
	    def.setType(RouteDstDef.TYPE_MASK); 
	    def.setDst(addr.getText());
	  }))
	 ({
	    if (needSmeId) def.setSmeId(getnameid("SME System id"));
	    cmd.addDstDef(def);
	 })
	;

/* ----------------------- Route command parsers ----------------------- */
route_src[RouteGenCommand cmd]
	: (OPT_SRC (srcdef[cmd])+)
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException(
        "Route srcdef missed or invalid. Syntax: src (subj <subject_name>|mask <mask>)+");
}

route_dst[RouteGenCommand cmd, boolean needSmeId]
	: (OPT_DST (dstdef[cmd, needSmeId])+)
	;
exception
catch [RecognitionException ex] {
   throw new RecognitionException(
	"Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask>"+
	((needSmeId) ? " <systemid>)+":")+"));
}

route_dm[RouteGenCommand cmd]
	: (VAL_DEF      { cmd.setDeliveryMode("default");  }
	  |VAL_STORE    { cmd.setDeliveryMode("store");    }
	  |VAL_FORWARD  { cmd.setDeliveryMode("forward");  }
	  |VAL_DATAGRAM { cmd.setDeliveryMode("datagram"); })
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Route delivery mode expected. Syntax: (default|store|forward|datagram)");
}

addroute returns [RouteAddCommand cmd] {
    cmd = new RouteAddCommand();
}
	: ({ cmd.setRoute(getnameid("Route name")); })
	  (OPT_NOTES { cmd.setNotes(getnameid("Subject notes")); })?
	  addroute_flags[cmd]
	  (OPT_SVCID { cmd.setServiceId(getint("serviceid")); })
	  (OPT_PRI   { cmd.setPriority(getint("priority")); })
	  (OPT_DM route_dm[cmd])?
	  (TGT_ACL    { cmd.setAclId(getlongid("acl id")); })?
	  (TGT_PROVIDER { cmd.setProviderName(getnameid("provider name")); })?
	  (TGT_CATEGORY { cmd.setCategoryName(getnameid("category name")); })?
	  (OPT_SRCSME { cmd.setSrcSmeId(getnameid("srcSmeId value"));   })?
	  (OPT_FWD    { cmd.setForwardTo(getnameid("forwardTo value")); })?
	  route_src[cmd]
	  route_dst[cmd, true]
	;

addroute_flags[RouteAddCommand cmd]
	: (OPT_ACTIVE 	          { cmd.setActive(true);   }
	  |OPT_INACTIVE	          { cmd.setActive(false);  })?
	  (OPT_HIDE               { cmd.setHide(true);     }
	  |OPT_NOHIDE             { cmd.setHide(false);    })?
	  (OPT_RP	(VAL_FORCE      { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_FORCE);    }
			|VAL_SUPPRESS   { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_SUPPRESS); }
		        |VAL_PASS       { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_PASS);     }))?
	  (OPT_BLOCKED (OPT_ALLOW { cmd.setAllowBlocked(true);   }
	               |OPT_DENY  { cmd.setAllowBlocked(false);  }))?
	  (OPT_FD                 { cmd.setForceDelivery(true);  })?
	  (OPT_TRANSIT            { cmd.setTransit(true);        })?
	  (OPT_BILL   	          { cmd.setBill(true);   	 }
	  |OPT_NOBILL 	          { cmd.setBill(false);  	 })
	  (OPT_ARCH   	          { cmd.setArc(true);    	 }
	  |OPT_NOARCH 	          { cmd.setArc(false);   	 })
	  (OPT_ALLOW  	          { cmd.setAllow(true);  	 }
	  |OPT_DENY   	          { cmd.setAllow(false); 	 })
	  (OPT_RCPT   	          { cmd.setReceipt(true);  }
	  |OPT_NORCPT 	          { cmd.setReceipt(false); })
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Route flags expected. "+
	"Syntax: [active|inactive] [hide|nohide] [forceReplayPath] [forceDelivery]"+
	"[transit] (bill|nobill) (arc|noarc) (allow|deny) (receipt|noreceipt)");
}

delroute returns [RouteDeleteCommand cmd] {
    cmd = new RouteDeleteCommand();
}
	: ({  cmd.setRoute(getnameid("Route name")); })
	;

altroute returns [RouteAlterCommand cmd] {
    cmd = new RouteAlterCommand();
    boolean addAction = true;
}
	: ({ cmd.setRoute(getnameid("Route name"));  })
	  (OPT_NOTES { cmd.setNotes(getnameid("Subject notes")); })?
	  altroute_flags[cmd]
	  (OPT_SVCID { cmd.setServiceId(getint("serviceid")); })?
	  (OPT_PRI   { cmd.setPriority(getint("priority"));   })?
	  (OPT_DM route_dm[cmd])?
	  (TGT_ACL     { cmd.setAclId(getlongid("acl id")); })?
	  (TGT_PROVIDER { cmd.setProviderName(getnameid("provider name")); })?
	  (TGT_CATEGORY { cmd.setCategoryName(getnameid("category name")); })?
	  (OPT_SRCSME  { cmd.setSrcSmeId(getnameid("srcSmeId value"));   })?
	  (OPT_FWD     { cmd.setForwardTo(getnameid("forwardTo value")); })?
	  (
		((ACT_ADD    { cmd.setAction(RouteAlterCommand.ACTION_ADD); addAction=true;  })
		|(ACT_DELETE { cmd.setAction(RouteAlterCommand.ACTION_DEL); addAction=false; }))
		 ( route_src[cmd] { 
			cmd.setTarget(RouteAlterCommand.TARGET_SRC);
		   }
		 | route_dst[cmd, addAction] { 
			cmd.setTarget(RouteAlterCommand.TARGET_DST); 
		   }
		 )
	  )?
	;

altroute_flags[RouteAlterCommand cmd]
	: (OPT_ACTIVE 	          { cmd.setActive(true);   }
	  |OPT_INACTIVE	          { cmd.setActive(false);  })?
	  (OPT_HIDE               { cmd.setHide(true);     }
	  |OPT_NOHIDE             { cmd.setHide(false);    })?
	  (OPT_RP	(VAL_FORCE      { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_FORCE);    }
			|VAL_SUPPRESS   { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_SUPPRESS); }
			|VAL_PASS       { cmd.setReplayPath(RouteGenCommand.REPLAY_PATH_PASS);     }))?
  	  (OPT_BLOCKED (OPT_ALLOW { cmd.setAllowBlocked(true);  }
	               |OPT_DENY  { cmd.setAllowBlocked(false); }))?
	  (OPT_FD (OPT_ON         { cmd.setForceDelivery(true); }
	          |OPT_OFF        { cmd.setForceDelivery(false); }))?
	  (OPT_TRANSIT (OPT_ON    { cmd.setTransit(true);  }
		       |OPT_OFF   { cmd.setTransit(false); }))?
	  (OPT_BILL   	          { cmd.setBill(true);     }
	  |OPT_NOBILL 	          { cmd.setBill(false);    })?
	  (OPT_ARCH   	          { cmd.setArc(true);      }
	  |OPT_NOARCH 	          { cmd.setArc(false);     })?
	  (OPT_ALLOW  	          { cmd.setAllow(true);    }
	  |OPT_DENY   	          { cmd.setAllow(false);   })?
	  (OPT_RCPT   	          { cmd.setReceipt(true);  }
	  |OPT_NORCPT 	          { cmd.setReceipt(false); })?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Route flags expected");
}

viewroute returns [RouteViewCommand cmd] {
    cmd = new RouteViewCommand();
}
	: ({ cmd.setRoute(getnameid("Route name")); })
	;

/* ----------------------- Alias command parsers ----------------------- */

addalias returns [AliasAddCommand cmd] {
    cmd = new AliasAddCommand();
}
	: (mask:STR     { cmd.setAlias(mask.getText());   })
	  (addr:STR     { cmd.setAddress(addr.getText()); })
	  (OPT_HIDE     { cmd.setHide(true);  }
	  |OPT_NOHIDE   { cmd.setHide(false); })?
	;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias mask expected");
}
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias address expected");
}

delalias returns [AliasDeleteCommand cmd] {
    cmd = new AliasDeleteCommand();
}
	: (mask:STR     { cmd.setAlias(mask.getText()); })
	;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias mask expected");
}

altalias returns [AliasAlterCommand cmd] {
    cmd = new AliasAlterCommand();
}
	: (mask:STR    { cmd.setAlias(mask.getText());   })
	  (addr:STR    { cmd.setAddress(addr.getText()); })
	  (OPT_HIDE    { cmd.setHide(true);  }
	  |OPT_NOHIDE  { cmd.setHide(false); })?
	;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias mask expected");
}
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias address expected");
}

viewalias returns [AliasViewCommand cmd] {
    cmd = new AliasViewCommand();
}
	: (mask:STR     { cmd.setAlias(mask.getText()); })
	;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Alias mask expected");
}

/* ----------------------- Subject command parsers --------------------- */

addsubj_mask[SubjectGenCommand cmd]
	: (mask:STR     { cmd.addMask(mask.getText()); })
	;
addsubj_masks[SubjectGenCommand cmd]
	: (addsubj_mask[cmd] (COMMA addsubj_mask[cmd])*)
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
}

addsubject returns [SubjectAddCommand cmd] {
    cmd = new SubjectAddCommand();
}
	: ({ cmd.setSubject(getnameid("Subject name")); })
	  (OPT_NOTES { cmd.setNotes(getnameid("Subject notes")); })?
	  ({ cmd.setDefaultSmeId(getnameid("SME id"));  })
	  addsubj_masks[cmd]
	;

altsubject returns [SubjectAlterCommand cmd] {
    cmd = new SubjectAlterCommand();
}
	: ({ cmd.setSubject(getnameid("Subject name")); })
	  (OPT_NOTES { cmd.setNotes(getnameid("Subject notes")); })?
	  (((ACT_ADD     { cmd.setActionAdd(); }
	    |ACT_DELETE  { cmd.setActionDelete(); }) 
	  addsubj_masks[cmd])
	  |(OPT_DEFSME { cmd.setDefaultSmeId(getnameid("SME id"));}))
	;

delsubject returns [SubjectDeleteCommand cmd] {
    cmd = new SubjectDeleteCommand();
}
	: ({ cmd.setSubject(getnameid("Subject name")); })
	;

viewsubject returns [SubjectViewCommand cmd] {
    cmd = new SubjectViewCommand();
}
	: ({ cmd.setSubject(getnameid("Subject name")); })
	;

/* ----------------------- Profile command parsers --------------------- */

profile_divert_opt[ProfileGenCommand cmd]
	: (OPT_ABSENT   { cmd.setDivertActiveAbsent(true);       })?
	  (OPT_BARRED   { cmd.setDivertActiveBarred(true);       })?
	  (OPT_BLOCKED  { cmd.setDivertActiveBlocked(true);      })?
	  (OPT_CAPACITY { cmd.setDivertActiveCapacity(true);     })?
	  (OPT_UNCONDIT { cmd.setDivertActiveUnconditional(true);})?
	  (OPT_MODIF    { cmd.setDivertModifiable(true);  }
	  |OPT_NOTMODIF { cmd.setDivertModifiable(false); })?		
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile devert options expected. Syntax: [absent][barred][blocked][capacity][unconditional] [modifiable|notmodifiable]");
}

profile_udh_concat_opt[ProfileGenCommand cmd] 
	: (OPT_UDHCONCAT (OPT_ON  { cmd.setUdhConcat(true);  }
	  	         |OPT_OFF { cmd.setUdhConcat(false); }))?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile udh concat option expected. Syntax: udhconcat on|off");
}

profile_translit_opt[ProfileGenCommand cmd] 
	: (OPT_TRANSLIT (OPT_ON  { cmd.setTranslit(true);  }
	  	        |OPT_OFF { cmd.setTranslit(false); }))?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile translit option expected. Syntax: translit on|off");
}

profile_alias_opt[ProfileGenCommand cmd] {
    cmd.setAliasOptions(true);
}
	: (OPT_HIDE     { cmd.setAliasHide(ProfileGenCommand.ALIAS_HIDE);    } 
	  |OPT_NOHIDE   { cmd.setAliasHide(ProfileGenCommand.ALIAS_NOHIDE);  }
	  |OPT_SUBSTIT	{ cmd.setAliasHide(ProfileGenCommand.ALIAS_SUBSTIT); })?
	  (OPT_MODIF    { cmd.setAliasModifiable(true);  }
	  |OPT_NOTMODIF { cmd.setAliasModifiable(false); })?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile alias options expected. Syntax: [hide|nohide|substitute] [modifiable|notmodifiable]");
}

profile_encode_opt[ProfileGenCommand cmd] {
    cmd.setUssd7Bit(false);
}
	: (VAL_DEF        { cmd.setGsm7Encoding(); }
	  |VAL_UCS2       { cmd.setUcs2Encoding(); }
	  |VAL_LATIN1     { cmd.setLatin1Encoding(); }
	  |VAL_UCS2LATIN1 { cmd.setUcs2Latin1Encoding(); })
	  (OPT_USSD7BIT   { cmd.setUssd7Bit(true); })?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile encoding options expected. Syntax: (default|ucs2|latin1|ucs2-latin1) [ussd7bit]");
}

profile_group_opt[ProfileGenCommand cmd]
    : (TGT_GROUP { cmd.setGroupIdOrName(getnameid("Group name or id")); })?
      (OPT_INPUTACCESSMASK { cmd.setInputAccessMask(getint("input access mask")); }
      |OPT_INPUTACCESSBIT
        (OPT_ON {cmd.setInputAccessBit(true, getint("input access bit"));}
        |OPT_OFF {cmd.setInputAccessBit(false, getint("input access bit"));} ))?

      (OPT_OUTPUTACCESSMASK { cmd.setOutputAccessMask(getint("output access mask")); }
      |OPT_OUTPUTACCESSBIT
              (OPT_ON {cmd.setOutputAccessBit(true, getint("output access bit"));}
              |OPT_OFF {cmd.setOutputAccessBit(false, getint("output access bit"));} ))?
    ;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Profile closed group options expected. Syntax: [group <groupName>|<groupId>] [inputAccessMask <intNum>|inputAccessBit <bitNum:0-31> <on|off>] [outputAccessMask <intNum>|outputAccessBit <bitNum:0-31> <on|off>]");
}

addprofile returns [ProfileAddCommand cmd] {
    cmd = new ProfileAddCommand();
}
: (mask:STR  { cmd.setMask(mask.getText());    })
  (OPT_REPORT (VAL_FULL  { cmd.setFullReport(); }
              |VAL_NONE  { cmd.setNoneReport(); }
              |VAL_FINAL { cmd.setFinalReport(); })
  )
  (OPT_LOCALE { cmd.setLocale(getnameid("Locale name")); } )
  (OPT_ENCODE profile_encode_opt[cmd] )?
  (TGT_ALIAS  profile_alias_opt[cmd]  )?
  (OPT_SPONSORED {cmd.setSponsored(getint("sponsored")); })?
  (OPT_DIVERT  { cmd.setDivertOptions(true); }
	      ({ cmd.setDivert(getnameid("Divert value")); })
	         profile_divert_opt[cmd] 
  )?
  profile_udh_concat_opt[cmd]
  profile_translit_opt[cmd]
  profile_group_opt[cmd]
  (OPT_SERVICESMASK { cmd.setServices(getint("services")); }
  |OPT_SERVICESBIT
      (OPT_ON {cmd.setServicesBit(true, getint("services bit"));}
      |OPT_OFF {cmd.setServicesBit(false, getint("services bit"));}
      )
  )?
;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Profile mask expected");
}
	
altprofile returns [ProfileAlterCommand cmd] {
    cmd = new ProfileAlterCommand();
}
	: (addr:STR   { cmd.setAddress(addr.getText()); })
	  (OPT_REPORT (VAL_FULL  { cmd.setFullReport(); }
	              |VAL_NONE  { cmd.setNoneReport(); }
	              |VAL_FINAL { cmd.setFinalReport(); }))?
	  (OPT_LOCALE { cmd.setLocale(getnameid("Locale name")); } )?
	  (OPT_ENCODE profile_encode_opt[cmd] )?
	  (TGT_ALIAS  profile_alias_opt[cmd]  )?
	  (OPT_SPONSORED {cmd.setSponsored(getint("sponsored")); })?
	  (OPT_DIVERT { cmd.setDivertOptions(true); }
	    ((OPT_SET   { cmd.setDivert(getnameid("Divert value")); })|
	     (OPT_CLEAR { cmd.setDivert(""); }))?
	    ((OPT_OFF   { cmd.setDivertActiveOn(false); })|
	     (OPT_ON    { cmd.setDivertActiveOn(true);  }))?
	    profile_divert_opt[cmd]
	  )?
	  profile_udh_concat_opt[cmd]
	  profile_translit_opt[cmd]
	  profile_group_opt[cmd]
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Profile address expected");
}

delprofile returns [ProfileDeleteCommand cmd] {
    cmd = new ProfileDeleteCommand();
}
	: (mask:STR  { cmd.setMask(mask.getText()); })
	;
exception[mask]
catch [RecognitionException ex] {
    throw new RecognitionException("Profile mask expected");
}

viewprofile returns [ProfileViewCommand cmd] {
    cmd = new ProfileViewCommand();
}
	: (addr:STR  { cmd.setAddress(addr.getText()); })
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Profile address expected");
}

/* ------------------ Distribution lists commands parsers ---------------- */
addprincipal returns [PrincipalAddCommand cmd] {
    cmd = new PrincipalAddCommand();
}
	: (addr:STR  { cmd.setAddress(addr.getText()); })
	  (OPT_NLIST { cmd.setMaxLists(getint("numlist"));    })
	  (OPT_NELEM { cmd.setMaxElements(getint("numelem")); })
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Principal address expected");
}
exception[numl]
catch [RecognitionException ex] {
    throw new RecognitionException("'numlist' integer value expected");
}
exception[nume]
catch [RecognitionException ex] {
    throw new RecognitionException("'numelem' integer value expected");
}
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Command invalid. Syntax: add principal <principal_address> numlist <number> numelem <number>");
}

altprincipal returns [PrincipalAlterCommand cmd] {
    cmd = new PrincipalAlterCommand();
}
	: (addr:STR  { cmd.setAddress(addr.getText()); })
	  (OPT_NLIST { cmd.setMaxLists(getint("numlist")); })?
	  (OPT_NELEM { cmd.setMaxElements(getint("numelem")); })?
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Principal address expected");
}
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Command invalid. Syntax: alter principal <principal_address> [numlist <number>] [numelem <number>]");
}

delprincipal returns [PrincipalDeleteCommand cmd] {
    cmd = new PrincipalDeleteCommand();
}
	: (addr:STR  { cmd.setAddress(addr.getText()); })
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Principal address expected");
}

viewprincipal returns [PrincipalViewCommand cmd] {
    cmd = new PrincipalViewCommand();
}
	: (addr:STR  { cmd.setAddress(addr.getText()); })
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Principal address expected");
}

adddl returns [DistributionListAddCommand cmd] {
    cmd = new DistributionListAddCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (OPT_NELEM { cmd.setMaxElements(getint("numelem")); })
	  (OPT_OWNER owner:STR { cmd.setOwner(owner.getText()); })?
	;
exception[owner]
catch [RecognitionException ex] {
    throw new RecognitionException("Owner address expected");
}
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Command invalid. Syntax: add dl <dl_name> [owner <owner_address>] numelem <number>");
}

altdl returns [DistributionListAlterCommand cmd] {
    cmd = new DistributionListAlterCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (OPT_NELEM { cmd.setMaxElements(getint("numelem")); })
	;

viewdl returns [DistributionListViewCommand cmd] {
    cmd = new DistributionListViewCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	;

deldl returns [DistributionListDeleteCommand cmd] {
    cmd = new DistributionListDeleteCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	;

adddlmember returns [MemberAddCommand cmd] {
    cmd = new MemberAddCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (member:STR   { cmd.setMember(member.getText());     })
	;
exception[member]
catch [RecognitionException ex] {
    throw new RecognitionException("Member address expected");
}

deldlmember returns [MemberDeleteCommand cmd] {
    cmd = new MemberDeleteCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (member:STR   { cmd.setMember(member.getText());     })
	;
exception[member]
catch [RecognitionException ex] {
    throw new RecognitionException("Member address expected");
}

adddlsubmitter returns [SubmitterAddCommand cmd] {
    cmd = new SubmitterAddCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (submitter:STR { cmd.setSubmitter(submitter.getText()); })
	;
exception[submitter]
catch [RecognitionException ex] {
    throw new RecognitionException("Submitter address expected");
}

deldlsubmitter returns [SubmitterDeleteCommand cmd] {
    cmd = new SubmitterDeleteCommand();
}
	: ({ cmd.setName(getnameid("Distribution list name")); })
	  (submitter:STR { cmd.setSubmitter(submitter.getText()); })
	;
exception[submitter]
catch [RecognitionException ex] {
    throw new RecognitionException("Submitter address expected");
}

/* ------------------ ACL commands parsers ---------------- */
addacl returns[AclAddCommand cmd] {
    cmd = new AclAddCommand();
} : (OPT_NAME  { cmd.setName (getnameid("acl name")); })
    (OPT_NOTES { cmd.setDescription(getnameid("acl notes")); })?
    (OPT_CACHE (VAL_FULL { cmd.setCache(AclGenCommand.CACHE_FULLCACHE); }
               |VAL_NONE { cmd.setCache(AclGenCommand.CACHE_DBSDIRECT); }))
  ;

delacl returns[AclDeleteCommand cmd] {
    cmd = new AclDeleteCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
  ;

altacl returns[AclAlterCommand cmd] {
    cmd = new AclAlterCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
    (OPT_NAME  { cmd.setName (getnameid("acl name")); })?
    (OPT_NOTES { cmd.setDescription(getnameid("acl notes")); })?
    (OPT_CACHE (VAL_FULL { cmd.setCache(AclGenCommand.CACHE_FULLCACHE); }
               |VAL_NONE { cmd.setCache(AclGenCommand.CACHE_DBSDIRECT); }))?
  ;

viewacl returns[AclViewCommand cmd] {
    cmd = new AclViewCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
  ;

grantacl returns[AclGrantCommand cmd] {
    cmd = new AclGrantCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
    (ADD_FOR addr:STR { cmd.setAddress(addr.getText()); })
  ;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Address string expected");
}

revokeacl returns[AclRevokeCommand cmd] {
    cmd = new AclRevokeCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
    (ADD_FOR addr:STR { cmd.setAddress(addr.getText()); })
  ;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Address string expected");
}

checkacl returns[AclCheckCommand cmd] {
    cmd = new AclCheckCommand();
} : ({ cmd.setAclId(getlongid("acl id"));   })
    (ADD_FOR addr:STR { cmd.setAddress(addr.getText()); })
  ;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Address string expected");
}

/* ------------------ SME commands parsers ---------------- */

sme_base_opt[SmeGenCommand cmd]
	: (OPT_MODE (VAL_TX   { cmd.setMode(SmeGenCommand.MODE_TX);   }
		    |VAL_RX   { cmd.setMode(SmeGenCommand.MODE_RX);   }
		    |VAL_TRX  { cmd.setMode(SmeGenCommand.MODE_TRX);  }))?
	  (OPT_TYPE (VAL_SMPP { cmd.setType(SmeGenCommand.TYPE_SMPP); }
		    |VAL_SS7  { cmd.setType(SmeGenCommand.TYPE_SS7);  }))?
	  (OPT_SME_N    { cmd.setSmeN(getint("smeN")); })?
	  (OPT_A_RANGE  addr:STR { cmd.setAddressRange(addr.getText()); })?
	  (OPT_PRI      { cmd.setPriority(getint("priority")); })?
	  (OPT_TON      { cmd.setTON(getint("ton")); })?
	  (OPT_NPI      { cmd.setNPI(getint("npi")); })?
	  (OPT_INT_V    { cmd.setInterfaceVersion(getint("interfaceVersion")); })?
	  (OPT_SYS_TYPE { cmd.setSystemType(getnameid("systemType")); })?
	  (OPT_PASSWORD { cmd.setPassword(getnameid("password")); })?
	  (OPT_TIMEOUT  { cmd.setTimeout(getint("timeout")); })?
	  (OPT_R_SCHEME { cmd.setReceiptScheme(getnameid("receiptScheme")); })?
	  (OPT_P_LIMIT  { cmd.setProclimit (getint("proclimit"));  })?
	  (OPT_S_LIMIT  { cmd.setSchedlimit(getint("schedlimit")); })?
      (OPT_ACCESSMASK { cmd.setAccessMask(getint("access mask")); }
      |OPT_ACCESSBIT
        (OPT_ON {cmd.setAccessBit(true, getint("access bit"));}
        |OPT_OFF {cmd.setAccessBit(false, getint("access bit"));} ))?
	;
exception[addr]
catch [RecognitionException ex] {
    throw new RecognitionException("Invalid addressRange value. Cause: "+ex.getMessage());
}

sme_add_opt[SmeGenCommand cmd]
	: (sme_base_opt[cmd])
	  (OPT_WANT_ALIAS { cmd.setWantAlias(true); })?
	  (OPT_FORCE_DC   { cmd.setForceDC  (true); })?
	  (OPT_DISABLED   { cmd.setDisabled (true); })?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Sme add option(s) invalid. Details: "+ex.getMessage());
}

sme_alt_opt[SmeGenCommand cmd]
	: (sme_base_opt[cmd])
	  (OPT_WANT_ALIAS (OPT_ON  { cmd.setWantAlias (true); }
			  |OPT_OFF { cmd.setWantAlias(false); }))?
	  (OPT_FORCE_DC   (OPT_ON  { cmd.setForceDC   (true); }
			  |OPT_OFF { cmd.setForceDC  (false); }))?
	  (OPT_DISABLED   (OPT_ON  { cmd.setDisabled  (true); }
			  |OPT_OFF { cmd.setDisabled (false); }))?
	  (ACT_DISCONNECT { cmd.setDisconnect(); })?
	;
exception
catch [RecognitionException ex] {
    throw new RecognitionException("Sme alt option(s) invalid. Details: "+ex.getMessage());
}

addsme returns [SmeAddCommand cmd] {
    cmd = new SmeAddCommand();
}	: ({ cmd.setSmeId(getnameid("SME id to add")); })
	  sme_add_opt[cmd]
	;
altsme returns [SmeAlterCommand cmd] {
    cmd = new SmeAlterCommand();
}	: ({ cmd.setSmeId(getnameid("SME id to alter")); })
	  sme_alt_opt[cmd]
	;
delsme returns [SmeDeleteCommand cmd] {
    cmd = new SmeDeleteCommand();
}	: ({ cmd.setSmeId(getnameid("SME id to delete")); })
	;
viewsme returns [SmeViewCommand cmd] {
    cmd = new SmeViewCommand();
}	: ({ cmd.setSmeId(getnameid("SME id to view")); })
	;

    /* ------------------ closed group commands parsers ---------------- */

    group_add_opt[ClosedGroupGenCommand cmd]
    	: (OPT_DESCR { cmd.setDescr(getnameid("closed group descr")); })?
    	;
    exception
    catch [RecognitionException ex] {
        throw new RecognitionException("closed group add option(s) invalid. Details: "+ex.getMessage());
    }

    group_alt_opt[ClosedGroupAlterCommand cmd]
    	: (OPT_RENAME { cmd.setClosedGroupNewName (getnameid("closed group name")); })?
    	  (OPT_DESCR { cmd.setDescr(getnameid("closed group descr")); })?
    	  (ACT_ADD { cmd.addMask(getnameid("closed group mask to add")); })*
    	  (ACT_DELETE { cmd.delMask(getnameid("closed group mask to delete")); })*
    	;
    exception
    catch [AdminException ex] {
        throw new RecognitionException("mask is not valid");
    }
    catch [RecognitionException ex] {
        throw new RecognitionException("closed group alt option(s) invalid. Details: "+ex.getMessage());
    }

    addgroup returns [ClosedGroupAddCommand cmd] {
        cmd = new ClosedGroupAddCommand();
    }	: ({ cmd.setClosedGroupName(getnameid("group id to add")); })
    	  group_add_opt[cmd]
    	;
    altgroup returns [ClosedGroupAlterCommand cmd] {
        cmd = new ClosedGroupAlterCommand();
    }	: ({ cmd.setClosedGroupId(getnameid("group id to alter")); })
    	  group_alt_opt[cmd]
    	;
    delgroup returns [ClosedGroupDeleteCommand cmd] {
        cmd = new ClosedGroupDeleteCommand();
    }	: ({ cmd.setClosedGroupId(getnameid("group id to delete")); })
    	;
    viewgroup returns [ClosedGroupViewCommand cmd] {
        cmd = new ClosedGroupViewCommand();
    }	: ({ cmd.setClosedGroupId(getnameid("group id to view")); })
    	;
/*--------------------Email SME ---------------------------- */
emailsme_gen_opt[EmailSmeGenCommand cmd]
    	: (OPT_TON { cmd.setTon(getint("emailsme ton")); })
    	  (OPT_NPI { cmd.setNpi(getint("emailsme npi")); })
    	  (OPT_ADDRESS { cmd.setAddress(getnameid("emailsme address")); })
    	;
exception
catch [RecognitionException ex] {
   throw new RecognitionException("emailsme option(s) invalid. Details: "+ex.getMessage());
}

viewemailsmebyid returns [EmailSmeLookupByUserIdCommand cmd] {
    cmd = new EmailSmeLookupByUserIdCommand();
}   : ( {cmd.setUserId(getnameid("name id")); } )
    ;
exception
catch [RecognitionException ex] {
   throw new RecognitionException("user id option invalid. Details: "+ex.getMessage());
}

viewemailsmebyaddr returns [EmailSmeLookupByAddressCommand cmd] {
    cmd = new EmailSmeLookupByAddressCommand();
}
    : emailsme_gen_opt[cmd]
    ;
exception
catch [RecognitionException ex] {
   throw new RecognitionException("ton, npi, address expected. Details: "+ex.getMessage());
}

addemailsme returns [EmailSmeAddCommand cmd] {
    cmd = new EmailSmeAddCommand();
}	: emailsme_gen_opt[cmd]
      (OPT_USERNAME { cmd.setUserName(getnameid("emailsme username")); }) ?
      (OPT_FORWARDEMAIL { cmd.setForwardEmail(getnameid("emailsme forwardemail")); })?
      (OPT_REALNAME { cmd.setRealName(getnameid("emailsme realname")); })?
      (OPT_LIMITTYPE { cmd.setLimitType(getnameid("emailsme limit type")); })
      (OPT_LIMITVALUE { cmd.setLimitValue(getint("emailsme limit value")); })
	;

delemailsme returns [EmailSmeDelCommand cmd] {
    cmd = new EmailSmeDelCommand();
}   : emailsme_gen_opt[cmd]
    ;
/* ------------------ Misc commands parsers ---------------- */

addprovider returns [ProviderAddCommand cmd] {
    cmd = new ProviderAddCommand();
}	: ({ cmd.setProviderId(getnameid("Provider id")); })
	;
delprovider returns [ProviderDeleteCommand cmd] {
    cmd = new ProviderDeleteCommand();
}	: ({ cmd.setProviderId(getnameid("Provider id")); })
	;

addcategory returns [CategoryAddCommand cmd] {
    cmd = new CategoryAddCommand();
}	: ({ cmd.setCategoryId(getnameid("Category id")); })
	;
delcategory returns [CategoryDeleteCommand cmd] {
    cmd = new CategoryDeleteCommand();
}	: ({ cmd.setCategoryId(getnameid("Category id")); })
	;
	
exportstats returns[StatExportCommand cmd] {
    cmd = new StatExportCommand();
}
	: (ADD_FOR { cmd.setDate(getnameid("Date to export"));        })
	  (ADD_TO  { cmd.setDriver(getnameid("Driver"));              }
		   { cmd.setSource(getnameid("Source"));              }
		   { cmd.setUser(getnameid("User"));                  }
		   { cmd.setPassword(getnameid("Password"));          } 
		   { cmd.setTablesPrefix(getnameid("tables prefix")); })?
	;
exportsms returns[SmsExportCommand cmd] {
    cmd = new SmsExportCommand();
}
	: (ADD_TO  { cmd.setDriver(getnameid("Driver"));        }
		   { cmd.setSource(getnameid("Source"));        }
		   { cmd.setUser(getnameid("User"));            }
		   { cmd.setPassword(getnameid("Password"));    }
		   { cmd.setTableName(getnameid("table name")); })?
	;
exportarchive returns[SmsArchiveExportCommand cmd] {
    cmd = new SmsArchiveExportCommand();
}
	: (ADD_FOR { cmd.setDate(getnameid("Date to export"));        })
	  (ADD_TO  { cmd.setDriver(getnameid("Driver"));    }
		   { cmd.setSource(getnameid("Source"));        }
		   { cmd.setUser(getnameid("User"));            }
		   { cmd.setPassword(getnameid("Password"));    }
		   { cmd.setTableName(getnameid("table name")); })?
	;
