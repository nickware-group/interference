
let Settings = {
    lang: 0,
    theme: 0,
};

function doSaveSettings() {
    localStorage.setItem("interlink_web_settings", JSON.stringify(Settings));
}

function doLoadSettings() {
    let settings = localStorage.getItem("interlink_web_settings");
    if (settings !== null) Settings = JSON.parse(settings);

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
        FManager.getCurrentFrame().getViewer().cs.style().clear();
        doApplyStylesheet(FManager.getCurrentFrame().getViewer().cs.style())
        FManager.getCurrentFrame().getViewer().cs.style().update();
        doApplyGrid(FManager.getCurrentFrame().getViewer().cs);
        doClearBackground();
    }
}

function doSwitchTheme() {
    if (Settings.theme === 0) Settings.theme = 1;
    else Settings.theme = 0;

    doApplyTheme();

    doSaveSettings();
}
