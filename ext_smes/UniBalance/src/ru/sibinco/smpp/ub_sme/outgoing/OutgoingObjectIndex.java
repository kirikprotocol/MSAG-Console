package ru.sibinco.smpp.ub_sme.outgoing;

class OutgoingObjectIndex {
    private int connectionId = -1;
    private int sequenceId = -1;

    public OutgoingObjectIndex(int connectionId, int sequenceId) {
        this.connectionId = connectionId;
        this.sequenceId = sequenceId;
    }

    public boolean equals(Object o) {
        if (!(o instanceof OutgoingObject)) return false;

        final OutgoingObject message = (OutgoingObject) o;

        if (connectionId != message.getOutgoingMessage().getConnectionId()) return false;
        if (sequenceId != message.getOutgoingMessage().getSequenceNumber()) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = connectionId;
        result = 29 * result + sequenceId;
        return result;
    }
}
