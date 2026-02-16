
facefullCreate(true);

window.addEventListener('load', function() {
    App();

    if (!facefull.isNative()) {
        doInitService();
    }
});

function doLevelSize(size) {
    size = parseFloat(size);
    let i;
    let type;

    for (i = 0; size > 1024; i++) {
        size /= 1024;
    }

    switch (i) {
        case 0: type = "B";  break;
        case 1: type = "KB"; break;
        case 2: type = "MB"; break;
        case 3: type = "GB"; break;
        case 4: type = "TB"; break;
    }

    return {size: Math.floor((size*10))/10, type: type};
}

function App() {
    facefull.doInit();

    facefull.Themes.setDefaultThemeName("Dark theme");
    facefull.Themes.doAttachThemeFile("Light theme", ["src/facefull/theme-light.min.css", "themes/style-light.css"]);
    doLoadSettings();

    facefull.Viewports.doAddDeviceDefinition("dashboard-min", 900);
    facefull.Viewports.doAddDeviceDefinition("mobile", 500);

    facefull.Viewports.doProcessRules();
    window.addEventListener("resize", function() {
        //facefull.Viewports.doProcessRules();
        try {
            document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.width = "auto";
            document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.height = "auto";
        } catch (e) {

        }
    });

    facefull.MainMenuBox.onPageOpen = function(name) {
        facefull.doUpdateAllScrollboxes();
    }

    facefull.doUpdateAllScrollboxes();

    FManager = new FrameManager();

    document.getElementById("CS1").children[0].style.width = "auto";
    document.getElementById("CS1").children[0].style.height = "auto";

    doSwitchLeftTopPanel(0, 0);
    doSwitchLeftBottomPanel(0, 0);
    doSwitchRightPanel(0, 0);

    facefull.Lists["NMDL"].onSelect = function(id) {
        FManager.getCurrentFrame().setAutoUpdateScope(false);
        FManager.getCurrentFrame().doManageViewport(id);
    };

    FManager.doSwitchCurrentFrame(0);
    document.getElementById("P0").style.zIndex = 0;

    let onObjectClick = function(viewer, id, list_name) {
        let name = facefull.Lists[list_name+(viewer+1)].elist.children[id].children[0].innerHTML;
        console.log("selection list", FManager.getFrame(viewer).selected_elements);

        FManager.getFrame(viewer).doClearSelectedElements();
        FManager.getFrame(viewer).getViewer().deselectAll();

        let data = FManager.getFrame(viewer).getViewer().getNodeData(name);

        if (data) {
            FManager.getFrame(viewer).getViewer().selectNode(name);
            FManager.getCurrentFrame().doAddSelectedElement(data.name, data.type);
            doClearParameterList(FManager.getCurrentFrameID());
            doCreateParameterList(data.name, NodeTypeNameByType[data.type]);
        }
    };

    let onObjectDoubleClick = function(viewer, id, list_name) {
        let name = facefull.Lists[list_name+(viewer+1)].elist.children[id].children[0].innerHTML;

        let data = FManager.getFrame(viewer).getViewer().getNodeData(name);
        if (data) {
            FManager.getFrame(viewer).getViewer().centerOnNode(name);
        }
    };

    let init_viewers = [0];
    for (let v in init_viewers) {
        facefull.Lists["EL"+(init_viewers[v]+1)].onSelect = function(id) {
            onObjectClick(init_viewers[v], id, "EL");
        };

        facefull.Lists["EL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            onObjectDoubleClick(init_viewers[v], id, "EL");
        }

        facefull.Lists["IL"+(init_viewers[v]+1)].onSelect = function(id) {
            onObjectClick(init_viewers[v], id, "IL");
        }

        facefull.Lists["IL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            onObjectDoubleClick(init_viewers[v], id, "IL");
        }

        facefull.Lists["OL"+(init_viewers[v]+1)].onSelect = function(id) {
            onObjectClick(init_viewers[v], id, "OL");
        }

        facefull.Lists["OL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            onObjectDoubleClick(init_viewers[v], id, "OL");
        }
    }

    document.getElementById("LT-B1").addEventListener("click", function() {
        doSwitchLeftTopPanel(FManager.getCurrentFrameID(), 0);
    });

    document.getElementById("LT-B2").addEventListener("click", function() {
        doSwitchLeftTopPanel(FManager.getCurrentFrameID(), 1)
    });

    document.getElementById("LT-B3").addEventListener("click", function() {
        doSwitchLeftTopPanel(FManager.getCurrentFrameID(), 2)
    });

    facefull.Counters["CV1"].onBeforeCount = function(direction) {
        if (facefull.Counters["CV1"].getValue()+direction > 5000) {
            facefull.Counters["CV1"].setValue(5000);
            return false;
        } else if (facefull.Counters["CV1"].getValue()+direction < 1) {
            facefull.Counters["CV1"].setValue(1);
            return false;
        }
        return true;
    }

    console.log("done init", performance.now());
    facefull.doEventSend("doWindowReady");
}
