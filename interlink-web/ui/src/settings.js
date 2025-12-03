
let Settings = {
    lang: "enUS",
    theme: 0,
    interlink_switch_tab: false,
    interlink_clear: false,
    interlink_default_host: "",
    interlink_default_iport: "",
    interlink_default_oport: "",
};

let ProjectSettings = {
    interlink: {
        host: "",
        iport: "",
        oport: "",
    }
}

function doOpenSettings() {
    AlertShowCustom("AS");
    facefull.Scrollboxes["STSB"].doUpdateScrollbar();
    doCheckSettingsDiff();
}

function setProjectSettings(data = null) {
    console.log("loading project settings", data)
    if (!data) {
        ProjectSettings.interlink.host = Settings.interlink_default_host;
        ProjectSettings.interlink.iport = Settings.interlink_default_iport;
        ProjectSettings.interlink.oport = Settings.interlink_default_oport;
        console.log("interlink default", ProjectSettings)
    } else {
        ProjectSettings = data;
        console.log("interlink settings from project", ProjectSettings)
    }

    document.getElementById("IH").value = ProjectSettings.interlink.host;
    document.getElementById("IIP").value = ProjectSettings.interlink.iport;
    document.getElementById("IOP").value = ProjectSettings.interlink.oport;

    doSyncInterlinkSettings();
}

facefull.doEventHandlerAttach("setCommonSettings", function(str) {
    let data = JSON.parse(str);
    Settings = data;

    document.getElementById("spis").checked = Settings.interlink_switch_tab === true;
    document.getElementById("spid").checked = Settings.interlink_clear === true;
    facefull.ItemPickers["DCP"].doSelect(Settings.theme);
});

function doCheckProjectSettingsDiff() {
    let state = ProjectSettings.interlink.host === document.getElementById("IH").value &&
                         ProjectSettings.interlink.iport === document.getElementById("IIP").value &&
                         ProjectSettings.interlink.oport === document.getElementById("IOP").value;
    return state;
}

function doCheckSettingsDiff() {
    // Settings.lang == document.getElementById("").value
    let state = doCheckProjectSettingsDiff() &&
                            Settings.interlink_switch_tab === document.getElementById("spis").checked &&
                            Settings.interlink_clear === document.getElementById("spid").checked &&
                            Settings.theme === facefull.ItemPickers["DCP"].getState();

    console.log("set apply state", state, Settings)
    if (state) document.getElementById("SBA").classList.add("Disabled");
    else document.getElementById("SBA").classList.remove("Disabled");
    return state;
}

function doSaveCloseSettings() {
    doSaveSettings();
    AlertHideCustom('AS');
}

function doSaveSettings() {
    if (!doCheckSettingsDiff()) {
        Settings.interlink_switch_tab = document.getElementById("spis").checked;
        Settings.interlink_clear = document.getElementById("spid").checked;
        Settings.theme = facefull.ItemPickers["DCP"].getState();
        facefull.doEventSend("doSaveSettings", JSON.stringify(Settings));
    }

    if (!doCheckProjectSettingsDiff()) {
        doSyncInterlinkSettings();
        setProjectChanged();
    }
}

function doCancelCloseSettings() {
    document.getElementById("spis").checked = Settings.interlink_switch_tab;
    document.getElementById("spid").checked = Settings.interlink_clear;
    facefull.ItemPickers["DCP"].doSelect(Settings.theme);
    document.getElementById("IH").value = ProjectSettings.interlink.host;
    document.getElementById("IIP").value = ProjectSettings.interlink.iport;
    document.getElementById("IOP").value = ProjectSettings.interlink.oport;
    AlertHideCustom('AS');
}

function doSyncInterlinkSettings() {
    let host = document.getElementById("IH").value;
    let iport = document.getElementById("IIP").value;
    let oport = document.getElementById("IOP").value;
    ProjectSettings.interlink.host = host;
    ProjectSettings.interlink.iport = iport;
    ProjectSettings.interlink.oport = oport;

    facefull.doEventSend("doSyncInterlinkSettings", host+"|"+iport+"|"+oport);
}

function getProjectSettings() {
    return ProjectSettings;
}
