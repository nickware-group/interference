
let Settings = {
    lang: 0,
    theme: 0,
    model_life_history_limit: 500,
};

function doSaveSettings() {
    localStorage.setItem("interlink_web_settings", JSON.stringify(Settings));
}

function doLoadSettings() {
    let settings = localStorage.getItem("interlink_web_settings");
    if (settings !== null) Settings = JSON.parse(settings);

    if (!Settings.model_life_history_limit) Settings.model_life_history_limit = 1000;

    doApplyTheme();
}

function doApplyTheme() {
    if (Settings.theme) {
        document.getElementById("TS").classList.remove("LightMode");
        document.getElementById("TS").classList.add("DarkMode");
    } else {
        document.getElementById("TS").classList.remove("DarkMode");
        document.getElementById("TS").classList.add("LightMode");
    }

    facefull.Themes.doApplyTheme(Settings.theme);
    if (FManager && FManager.getCurrentFrame()) {
        doApplyStylesheet(FManager.getCurrentFrame().getViewer())
    }
}

function doSwitchTheme() {
    if (Settings.theme === 0) Settings.theme = 1;
    else Settings.theme = 0;

    doApplyTheme();

    doSaveSettings();
}

function doOpenSettingsWindow() {
    facefull.Counters["CV1"].setValue(getModelHistoryLimit());
    AlertShowCustom("ASW");
}

function doCloseSettingsWindow() {
    AlertHideCustom("ASW");
}

function doSaveCloseSettingsWindow() {
    setModelHistoryLimit(facefull.Counters["CV1"].getValue());
    AlertHideCustom("ASW");
    doSaveSettings();
}

function setModelHistoryLimit(value) {
    Settings.model_life_history_limit = value;
}

function getModelHistoryLimit() {
    return Settings.model_life_history_limit;
}
