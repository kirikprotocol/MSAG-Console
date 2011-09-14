package mobi.eyeline.dcpgw;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 06.07.11
 * Time: 16:42
 */
public enum FinalMessageState {

    DELIVERED { public String toString(){return "DELIVRD"; } },
    EXPIRED,
    DELETED,
    UNDELIVERABLE { public String toString(){return "UNDELIV";} }    ,
    ACCEPTED { public String toString() {return "ACCEPTD";} },
    UNKNOWN,
    REJECTED { public String toString(){return "REJECTD";} }

}
