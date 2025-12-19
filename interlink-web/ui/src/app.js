
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
        FManager.getCurrentFrame().doManageViewport(id);
    };

    MouseLinkFrom = "";
    NeuronForCopy = "";
    doClearBackground();
    FManager.doSwitchCurrentFrame(0);
    document.getElementById("P0").style.zIndex = 0;

    let init_viewers = [0];
    for (let v in init_viewers) {
        facefull.Lists["EL"+(init_viewers[v]+1)].onSelect = function(id) {
            let name = facefull.Lists["EL"+(init_viewers[v]+1)].elist.children[id].children[0].innerHTML;
            console.log("selection list", FManager.getFrame(init_viewers[v]).selected_elements)
            let list = [...FManager.getFrame(init_viewers[v]).selected_elements];
            for (let i in list) {
                FManager.getFrame(init_viewers[v]).getViewer().cs.filter('[id = "'+list[i]+'"]').unselect();
            }

            // FManager.getFrame(init_viewers[v]).doClearSelectedElements();
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');

            if (obj.length > 0) {
                obj.select();
                doCreateParameterList(name);
            } else {
                doClearParameterList(FManager.getCurrentFrameID());
                doCreateParameterList("", "background");
            }
        };

        facefull.Lists["EL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            let name = facefull.Lists["EL"+(init_viewers[v]+1)].elist.children[id].children[0].innerHTML;
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');
            if (obj.length > 0) {
                let position = obj.renderedPosition();
                let cs_width = FManager.getFrame(init_viewers[v]).getViewer().cs.width();
                let cs_height = FManager.getFrame(init_viewers[v]).getViewer().cs.height();
                let current_pan = FManager.getFrame(init_viewers[v]).getViewer().cs.pan();
                let px= current_pan.x - position.x + cs_width/2
                let py= current_pan.y - position.y + cs_height/2
                FManager.getFrame(init_viewers[v]).getViewer().cs.pan({ x: px, y: py });
            }
        }

        facefull.Lists["IL"+(init_viewers[v]+1)].onSelect = function(id) {
            let name = facefull.Lists["IL"+(init_viewers[v]+1)].elist.children[id].children[0].innerHTML;
            console.log("selection list", FManager.getFrame(init_viewers[v]).selected_elements)
            let list = [...FManager.getFrame(init_viewers[v]).selected_elements];
            for (let i in list) {
                FManager.getFrame(init_viewers[v]).getViewer().cs.filter('[id = "'+list[i]+'"]').unselect();
            }

            // FManager.getFrame(init_viewers[v]).doClearSelectedElements();
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');

            if (obj.length > 0) {
                obj.select();
                doCreateParameterList(name, "entry");
            } else {
                doClearParameterList(FManager.getCurrentFrameID());
            }
        }

        facefull.Lists["IL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            let name = facefull.Lists["IL"+(init_viewers[v]+1)].elist.children[id].children[0].innerHTML;
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');
            if (obj.length > 0) {
                let position = obj.renderedPosition();
                let cs_width = FManager.getFrame(init_viewers[v]).getViewer().cs.width();
                let cs_height = FManager.getFrame(init_viewers[v]).getViewer().cs.height();
                let current_pan = FManager.getFrame(init_viewers[v]).getViewer().cs.pan();
                console.log("d click", position, cs_width, cs_height);
                let px= current_pan.x - position.x + cs_width/2
                let py= current_pan.y - position.y + cs_height/2
                FManager.getFrame(init_viewers[v]).getViewer().cs.pan({ x: px, y: py });
            }
        }

        facefull.Lists["OL"+(init_viewers[v]+1)].onSelect = function(id) {
            let name = FManager.getFrame(init_viewers[v]).getData().output_list[id].id;
            console.log("selection list", FManager.getFrame(init_viewers[v]).selected_elements)
            let list = [...FManager.getFrame(init_viewers[v]).selected_elements];
            for (let i in list) {
                FManager.getFrame(init_viewers[v]).getViewer().cs.filter('[id = "'+list[i]+'"]').unselect();
            }

            // FManager.getFrame(init_viewers[v]).doClearSelectedElements();
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');

            if (obj.length > 0) {
                obj.select();
                doCreateParameterList(name, "output");
            } else {
                doClearParameterList(FManager.getCurrentFrameID());
            }
        }

        facefull.Lists["OL"+(init_viewers[v]+1)].onDoubleClick = function(id) {
            let name = FManager.getFrame(init_viewers[v]).getData().output_list[id].id;
            let obj = FManager.getFrame(init_viewers[v]).getViewer().cs.filter('node[id = "'+name+'"]');
            if (obj.length > 0) {
                let position = obj.renderedPosition();
                let cs_width = FManager.getFrame(init_viewers[v]).getViewer().cs.width();
                let cs_height = FManager.getFrame(init_viewers[v]).getViewer().cs.height();
                let current_pan = FManager.getFrame(init_viewers[v]).getViewer().cs.pan();
                console.log("d click", position, cs_width, cs_height);
                let px= current_pan.x - position.x + cs_width/2
                let py= current_pan.y - position.y + cs_height/2
                FManager.getFrame(init_viewers[v]).getViewer().cs.pan({ x: px, y: py });
            }
        }
    }

    console.log("done init", performance.now());
    facefull.doEventSend("doWindowReady");
}
