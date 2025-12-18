
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
    // facefull.Locales.doAttachLocaleFile("English", ["../locale/locale-en.css"]);
    // facefull.Locales.doAttachLocaleFile("Russian", ["../locale/locale-ru.css"]);
    // facefull.Locales.doAttachLocaleFile("German", ["../locale/locale-de.css"]);

    facefull.Themes.setDefaultThemeName("Dark theme");
    facefull.Themes.doAttachThemeFile("Light theme", ["src/facefull/theme-light.min.css", "themes/style-light.css"]);
    // facefull.ItemPickers["DCP"].onSelect = function(id) {
    //     console.log("selected theme", id);
    //     facefull.Themes.doApplyTheme(id);
    //
    //     FManager.getFrame(0).getViewer().cs.style().clear();
    //     doApplyStylesheet(FManager.getFrame(0).getViewer().cs.style())
    //     FManager.getFrame(0).getViewer().cs.style().update();
    //     doApplyGrid(FManager.getFrame(0).getViewer().cs);
    //
    //     FManager.getInterlinkFrame().getViewer().cs.style().clear();
    //     doApplyStylesheet(FManager.getInterlinkFrame().getViewer().cs.style())
    //     FManager.getInterlinkFrame().getViewer().cs.style().update();
    //     doApplyGrid(FManager.getInterlinkFrame().getViewer().cs);
    //
    //     doClearBackground();
    // }
    doLoadSettings();

    facefull.Viewports.doAddDeviceDefinition("dashboard-min", 900);
    facefull.Viewports.doAddDeviceDefinition("mobile", 500);

    facefull.Viewports.doAddRule("dashboard-min", function(isactive) {
        if (isactive) {
            //document.getElementById("MPW").classList.remove("CenterWrapper");
            //document.getElementById("MP").classList.remove("Grid");
            //document.getElementById("MP").classList.remove("MainPanels");
        } else {
            //document.getElementById("MPW").classList.add("CenterWrapper");
            //document.getElementById("MP").classList.add("Grid");
            //document.getElementById("MP").classList.add("MainPanels");
        }
    });
    // facefull.Viewports.doAddRule("mobile", function(isactive) {
    //     let popupmenus = document.getElementsByClassName("PopupMenu");
    //     if (isactive) {
    //         document.getElementById("G").classList.add("Mobile");
    //         document.getElementById("G").classList.add("Touch");
    //         document.getElementById("TT").classList.add("Touch");
    //         document.getElementById("AE").classList.add("Mobile");
    //         Array.from(popupmenus).forEach((el) => {
    //             el.classList.add("Mobile");
    //         });
    //     } else {
    //         document.getElementById("G").classList.remove("Mobile");
    //         document.getElementById("G").classList.remove("Touch");
    //         document.getElementById("TT").classList.remove("Touch");
    //         document.getElementById("AE").classList.remove("Mobile");
    //         Array.from(popupmenus).forEach((el) => {
    //             el.classList.remove("Mobile");
    //         });
    //     }
    // });

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

    facefull.Locales.getLocaleList().forEach(locale => {
        // facefull.Comboboxes["LD"].doAddItem(locale.localename);
    });

    facefull.Themes.getThemeList().forEach(theme => {
        // facefull.Comboboxes["SD"].doAddItem(theme.themename);
    });

    FManager = new FrameManager();

    document.getElementById("CS1").children[0].style.width = "auto";
    document.getElementById("CS1").children[0].style.height = "auto";

    doSwitchLeftTopPanel(0, 0);
    doSwitchLeftBottomPanel(0, 0);
    doSwitchRightPanel(0, 0);

    facefull.Lists["NMDL"].onSelect = function(id) {
        FManager.getCurrentFrame().doManageViewport(id);
        // doInitMetricsShowRanges();
        // let range = NeuronMetricsRanges[facefull.Comboboxes["MRCB"].getState()];
        // facefull.doEventSend("doLoadMetrics", CurrentSelectedNeuron+"|"+id+"|"+range[0]+"|"+range[1]);
    };

    // doInitHotkeys();

    // facefull.Tabs["VMT"].onTabChanged = function(num) {
    //     MouseLinkFrom = "";
    //     NeuronForCopy = "";
    //     doClearBackground();
    //     FManager.doSwitchCurrentFrame(num-1);
    //     for (let i = 0; i <= 4; i++) {
    //         document.getElementById("P"+i).style.zIndex = 0;
    //     }
    //     document.getElementById("P"+num).style.zIndex = 10;
    // }

    // facefull.Tabs["VMT"].doSelectTab(0);

    // facefull.ItemPickers["CPIP"].onSelect = function(id) {
    //     doChangeMouseMode(id);
    // }

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

            if (FManager.getCurrentFrameID() === FManager.getInterlinkFrameID()) {
                doInitMetricsShowRanges();
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

    console.log("done app", performance.now());
    facefull.doEventSend("doWindowReady");

    // doCreateParameterList("", "background", FManager.getInterlinkFrameID());
    // AlertShowCustom("AL");
}

function doAppExit() {
    facefull.doEventSend("doWindowClose");
}

function doAppExitForce() {
    facefull.doEventSend("doWindowCloseForce");
}

function doInitHotkeys() {
    facefull.HotkeyHolders["DGHH"].setHotkey('D', false, true, false);
    facefull.HotkeyHolders["DGHH"].onHotkey = function(hotkey) {
        AlertShow("Confirm", "Do you want to delete this element?", "warning", "YESNO", [function() {
            let name = FManager.getCurrentFrame().getLastSelectedElement();
            let edge = FManager.getCurrentFrame().getViewer().cs.edges('[id = "'+name+'"]');
            let node = FManager.getCurrentFrame().getViewer().cs.nodes('[id = "'+name+'"]');
            if (edge.length) {
                console.log("delete ")
                let link = name.split("-");
                if (link.length >= 2)
                    doDeleteLink(link[0], link[1]);
            } else if (node.length) {
                doRemoveElement([name]);
            }
        }, function(){}]);
    }

    facefull.HotkeyHolders["CGHH"].setHotkey('C', false, true, false);
    facefull.HotkeyHolders["CGHH"].onHotkey = function(hotkey) {
        doCheckNeuronForCopy(FManager.getCurrentFrame().selected_elements, FManager.getCurrentFrameID());
    }

    facefull.HotkeyHolders["VGHH"].setHotkey('V', false, true, false);
    facefull.HotkeyHolders["VGHH"].onHotkey = function(hotkey) {
        doPasteElement();
    }
}
