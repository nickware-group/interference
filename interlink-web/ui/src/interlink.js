
let InterlinkState = 0;
let InterlinkMode = -1;

facefull.doEventHandlerAttach("setInterlinkState", function(str) {
    let eip = document.getElementById("IP");
    if (str === "1") {
        InterlinkState = 2;
        eip.classList.add("Connected");
        if (Settings.interlink_clear) {
            doDeleteAllMetrics();
        }
    } else {
        InterlinkState = 1;
        eip.classList.remove("Connected");
        eip.children[5].classList.add("Infinity");
        eip.children[5].innerHTML = " ms";
    }
});

facefull.doEventHandlerAttach("setInterlinkPing", function(str) {
    let eip = document.getElementById("IP");
    eip.children[5].classList.remove("Infinity");
    eip.children[5].innerHTML = str+"ms";
});

function doRestartInterlink() {
    let eip = document.getElementById("IP");
    eip.classList.add("Restart");
    let structure = "";
    if (InterlinkMode === 1) {
        let data = getStructure();
        structure = JSON.stringify(data);
    }
    facefull.doEventSend("doRestartInterlink", structure);
    setTimeout(function() {
        eip.classList.remove("Restart");
    }, 1500);
}

function doStartStopInterlink(structure = "") {
    let eip = document.getElementById("IP");
    if (!InterlinkState) {
        InterlinkState = 1;
        InterlinkMode = 0;
        eip.classList.add("Active");
        eip.classList.remove("Connected");
        facefull.doEventSend("doStartInterlink", structure);
        eip.children[5].classList.add("Infinity");
        eip.children[5].innerHTML = " ms";
        eip.children[5].classList.remove("Hidden");
        eip.children[1].classList.remove("Hidden");
        eip.children[2].classList.add("Hidden");
        if (Settings.interlink_switch_tab)
            facefull.Tabs["VMT"].doSelectTab(3);
    } else {
        InterlinkState = 0;
        eip.classList.remove("Active");
        eip.classList.remove("Connected");
        eip.children[5].classList.add("Hidden");
        eip.children[5].classList.remove("Infinity");
        eip.children[5].innerHTML = "";
        eip.children[1].classList.add("Hidden");
        eip.children[2].classList.remove("Hidden");
        facefull.doEventSend("doStopInterlink");
    }
}

function doStartInterlinkStructure() {
    let data = getStructure();
    doStartStopInterlink(JSON.stringify(data));
    InterlinkMode = 1;
}

function doOpenInterlinkSettings() {
    AlertShowCustom("AS");
    facefull.Scrollboxes["STSB"].doUpdateScrollbar();
    let eisl = document.getElementById("ISL");
    facefull.Scrollboxes["STSB"].setScrollPosition(eisl.offsetTop);
}

function setInterlinkConnectionName(name) {
    document.getElementById("ICN").innerHTML = name;
    document.getElementById("ICH").innerHTML = window.location.hostname;
}

function setInterlinkConnectionStatus(connected) {
    if (connected === !document.getElementById("IC").classList.contains("Disconnected")) return;

    if (connected) {
        document.getElementById("IC").classList.remove("Disconnected");
        document.getElementById("IC").setAttribute("data-tooltip-text", "Interlink Web connection (connected)");
    } else {
        document.getElementById("IC").classList.add("Disconnected");
        document.getElementById("IC").setAttribute("data-tooltip-text", "Interlink Web connection (disconnected)");
    }
}
