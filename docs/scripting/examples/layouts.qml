import QtQml 2.0
import QOwnNotesTypes 1.0

Script {
    /**
     * This function is called when layouts are switched
     *
     * @param oldUuid old uuid of layout
     * @param newUuid new uuid of layout
     */
    function layoutSwitchedHook(oldUuid, newUuid) {
        script.log(oldUuid);
        script.log(newUuid);
    }
}
