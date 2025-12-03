
facefull.setNative(false);

function doInitService() {
    let settings = {
        app_settings: window.localStorage.getItem('app_settings'),
        parameters: window.localStorage.getItem('parameters')
    };

    if (!settings.app_settings) {
        settings.app_settings = JSON.stringify({"style":1});
    }

    if (!settings.parameters) {
        settings.parameters = JSON.stringify({

        });
    }

    facefull.doEventSend("onLoadSettings", JSON.stringify(settings),{type: "local"});

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
        event_err: "onServiceReconnect"
    });
}

function doUpdateStructure() {
    facefull.doEventSend("update_structure", "", {
        type: "backend",
        event_ok: "doProcessData",
        event_err: "onServiceReconnect"
    });
}

function doUpdateData() {
    facefull.doEventSend("update_data", "", {
        type: "backend",
        event_ok: "doUpdateData",
        event_err: "onServiceReconnect"
    });
}
