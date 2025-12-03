
let ProjectChanged = false;

facefull.doEventHandlerAttach("doLoadProject", function(str) {
    doLoadProject(str);
    facefull.ItemPickers["CPIP"].doSelect(0);
    doRefreshProjectStats();
});

facefull.doEventHandlerAttach("doCreateNewProject", function(str) {
    doCreateProject();
    facefull.ItemPickers["CPIP"].doSelect(0);
});

facefull.doEventHandlerAttach("doRefreshProjectStats", function(str) {
    doRefreshProjectStats(str);
});

facefull.doEventHandlerAttach("doCheckProjectChanges", function(str) {
    if (doCheckProjectChanges()) {
        AlertShowCustom("AAC");
    } else {
        doAppExitForce();
    }
});

function doOpenCreateProjectDialog() {
    AlertShow("Create new structure", "Do you really want to close current project and create a new one?", "warning", "YESNO", [function() {
        doCreateProject();
    }, function(){}]);
}

function doCreateProject() {
    console.log("doCreateProject");
    FManager.getFrame(0).doUpdateViewer();
    FManager.getFrame(0).doClearLists();
    facefull.Tabs["VMT"].doSelectTab(0);
    doClearParameterList();
    doRefreshProjectStats("Untitled project");
    setProjectSettings();
    setTimeout(function () {
        facefull.doEventSend("doCreateNewProject");
    },100);
    doCreateParameterList("", "background", 0);
}

function doRefreshProjectStats(name = "") {
    let edges = FManager.getFrame(0).getViewer().cs.edges('');
    let onodes = FManager.getFrame(0).getViewer().cs.filter('node[weight = 2]');
    let ne_list = {};
    for (let n in FManager.getFrame(0).getData().neuron_list) {
        let ensemble = FManager.getFrame(0).getData().neuron_list[n].ensemble;
        if (!ne_list[ensemble]) ne_list[ensemble] = [];
        ne_list[ensemble].push(FManager.getFrame(0).getData().neuron_list[n].name);
    }

    let ssize = doLevelSize(getStructureSize(0));
    // if (name !== "") document.getElementById("PS-PN").innerHTML = name;
    // document.getElementById("PS-SS").innerHTML = ssize.size+" "+ssize.type;
    // document.getElementById("PS-E").innerHTML = Object.keys(ne_list).length;
    // document.getElementById("PS-EN").innerHTML = FManager.getFrame(0).getData().entry_list.length.toString();
    // document.getElementById("PS-O").innerHTML = onodes.length;
    // document.getElementById("PS-N").innerHTML =  Object.keys(FManager.getFrame(0).getData().neuron_list).length.toString();
    // document.getElementById("PS-L").innerHTML = edges.length;
}

function doRefreshInterlinkStats() {
    let edges = FManager.getInterlinkFrame().getViewer().cs.edges('');
    let onodes = FManager.getInterlinkFrame().getViewer().cs.filter('node[weight = 2]');
    let ne_list = {};
    for (let n in FManager.getInterlinkFrame().getData().neuron_list) {
        let ensemble = FManager.getInterlinkFrame().getData().neuron_list[n].ensemble;
        if (!ne_list[ensemble]) ne_list[ensemble] = [];
        ne_list[ensemble].push(FManager.getInterlinkFrame().getData().neuron_list[n].name);
    }

    let ssize = doLevelSize(getStructureSize(FManager.getInterlinkFrameID()));
    document.getElementById("IS-SS").innerHTML = ssize.size+" "+ssize.type;
    document.getElementById("IS-E").innerHTML = Object.keys(ne_list).length;
    document.getElementById("IS-EN").innerHTML = FManager.getInterlinkFrame().getData().entry_list.length.toString();
    document.getElementById("IS-O").innerHTML = onodes.length;
    document.getElementById("IS-N").innerHTML =  Object.keys(FManager.getInterlinkFrame().getData().neuron_list).length.toString();
    document.getElementById("IS-L").innerHTML = edges.length;
}

function doOpenProject() {
    facefull.doEventSend("doOpenProject");
}

function doLoadProject(datastr) {
    let data = JSON.parse(datastr);
    setProjectSettings(data.project_settings);
    doRefreshData(0, data.project_data);
}

function doImportStructure() {
    facefull.doEventSend("doLoadData");
}

function doExportStructure() {
    AlertShow("Exporting", "Exporting neural network structure...", "info", "");
    let data = getStructure();
    console.log("export", data);
    facefull.doEventSendEx("doExportData", JSON.stringify(data));
}

function getProjectData() {
    let elements = FManager.getFrame(0).getViewer().cs.json();
    let data = {
        entry_list: FManager.getFrame(0).getData().entry_list,
        neuron_list: FManager.getFrame(0).getData().neuron_list,
        output_list: FManager.getFrame(0).getData().output_list,
        network_info: FManager.getFrame(0).getData().network_info,
        viewer: elements,
    };
    console.log("get project", data);
    return data;
}

function doSaveProject() {
    let project_info = {
        project_data: getProjectData(),
        project_settings: getProjectSettings()
    }
    console.log("saving project", project_info.project_settings)
    facefull.doEventSendEx("doSaveProject", JSON.stringify(project_info));
    setProjectNotChanged();
}

function doSaveProjectAs() {
    let project_info = {
        project_data: getProjectData(),
        project_settings: getProjectSettings()
    }
    facefull.doEventSendEx("doSaveProjectAs", JSON.stringify(project_info));
    setProjectNotChanged();
}

function doSaveProjectAndExit() {
    let project_info = {
        project_data: getProjectData(),
        project_settings: getProjectSettings()
    }
    facefull.doEventSendEx("doSaveProjectAndExit", JSON.stringify(project_info));
}

function doCheckProjectChanges() {
    return ProjectChanged;
}

function setProjectChanged() {
    ProjectChanged = true;
    // facefull.doEventSend("setProjectChanged");
    // console.trace("setProjectChanged");
}

function setProjectNotChanged() {
    ProjectChanged = false;
}
