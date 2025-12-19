
facefull.setNative(false);

facefull.doEventHandlerAttach("onHostDisconnect", function(str) {
    setInterlinkConnectionStatus(false);
});

function doInitService() {
    doLoadStructure();

    setInterval(function() {
        doUpdateStructure();
        doUpdateData();
    }, 3000);
}

function doLoadStructure() {
    facefull.doEventSend("load_structure", "", {
        type: "backend",
        event_ok: "doProcessData",
        event_err: "onHostDisconnect"
    });
}

function doUpdateStructure() {
    facefull.doEventSend("update_structure", "", {
        type: "backend",
        event_ok: "doProcessData",
        event_err: "onHostDisconnect"
    });
}

function doUpdateData() {
    facefull.doEventSend("update_data", "", {
        type: "backend",
        event_ok: "doUpdateData",
        event_err: "onHostDisconnect"
    });
}
