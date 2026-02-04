let ScopesEditList = [];
let PhantomEditList = [];

function doLPTId2Eid(viewer, id) {
    let eid = "";
    switch (id) {
        case 0:
            eid = "ELP"+(viewer+1);
            break;
        case 1:
            eid = "ILP"+(viewer+1);
            break;
        case 2:
            eid = "OLP"+(viewer+1);
            break;
    }
    return eid;
}

function doLPBId2Eid(viewer, id) {
    let eid = "";
    switch (id) {
        case 0:
            eid = "DLP"+(viewer+1);
            break;
    }
    return eid;
}

function doSwitchLeftTopPanel(viewer, id) {
    let etp = document.getElementById("LTP"+(viewer+1));
    let etbp = document.getElementById("DLP1");

    let eid = doLPTId2Eid(viewer, id);
    let e = document.getElementById(eid);
    if (FManager.getFrame(viewer).getLeftPanelTopState() === id) {
        e.classList.add("Hidden");
        etp.children[id].classList.remove("Selected");
        FManager.getFrame(viewer).setLeftPanelTopState(-1);
        etbp.classList.remove("BottomList");
        facefull.doUpdateAllScrollboxes();
        return;
    }

    if (FManager.getFrame(viewer).getLeftPanelTopState() !== -1) {
        document.getElementById(doLPTId2Eid(viewer, FManager.getFrame(viewer).getLeftPanelTopState())).classList.add("Hidden");
        etp.children[FManager.getFrame(viewer).getLeftPanelTopState()].classList.remove("Selected");
    } else if (FManager.getFrame(viewer).getLeftPanelBottomState() === -1) {
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.width = "auto";
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.height = "auto";
    }
    etp.children[id].classList.add("Selected");
    e.classList.remove("Hidden");
    etbp.classList.add("BottomList");
    FManager.getFrame(viewer).setLeftPanelTopState(id);
    facefull.doUpdateAllScrollboxes();
}

function doSwitchLeftBottomPanel(viewer, id) {
    let etp = document.getElementById("LTP"+(viewer+1));
    let top_count = FManager.getFrame(viewer).getLeftPanelTopCount();

    let eid = doLPBId2Eid(viewer, id);
    let e = document.getElementById(eid);

    console.log(eid, FManager.getFrame(viewer).getLeftPanelBottomState(), id)

    if (FManager.getFrame(viewer).getLeftPanelBottomState() === id) {
        e.classList.add("Hidden");
        etp.children[id+top_count].classList.remove("Selected");
        FManager.getFrame(viewer).setLeftPanelBottomState(-1);
        facefull.doUpdateAllScrollboxes();
        return;
    }

    if (FManager.getFrame(viewer).getLeftPanelBottomState() !== -1) {
        document.getElementById(doLPBId2Eid(viewer, FManager.getFrame(viewer).getLeftPanelBottomState())).classList.add("Hidden");
        etp.children[FManager.getFrame(viewer).getLeftPanelBottomState()+top_count].classList.remove("Selected");
    } else if (FManager.getFrame(viewer).getLeftPanelTopState() === -1) {
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.width = "auto";
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.height = "auto";
    }
    etp.children[id+top_count].classList.add("Selected");
    e.classList.remove("Hidden");
    FManager.getFrame(viewer).setLeftPanelBottomState(id);
    facefull.doUpdateAllScrollboxes();
}

function doRPId2Eid(viewer, id) {
    let eid = "";
    switch (id) {
        case 0:
            eid = "DRP"+(viewer+1);
            break;
    }
    return eid;
}

function doSwitchRightPanel(viewer, id) {
    let etp = document.getElementById("RTP"+(viewer+1));

    let eid = doRPId2Eid(viewer, id);
    let e = document.getElementById(eid);
    if (FManager.getFrame(viewer).getRightPanelState() === id) {
        e.classList.add("Hidden");
        etp.children[id].classList.remove("Selected");
        FManager.getFrame(viewer).setRightPanelState(-1);
        facefull.doUpdateAllScrollboxes();
        return;
    }

    if (FManager.getFrame(viewer).getRightPanelState() !== -1) {
        document.getElementById(doRPId2Eid(viewer, FManager.getFrame(viewer).getRightPanelState() )).classList.add("Hidden");
        etp.children[FManager.getFrame(viewer).getRightPanelState()].classList.remove("Selected");
    } else {
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.width = "auto";
        document.getElementById("CS"+(FManager.getCurrentFrameID()+1)).children[0].style.height = "auto";
    }
    etp.children[id].classList.add("Selected");
    e.classList.remove("Hidden");
    FManager.getFrame(viewer).setRightPanelState(id);
    facefull.doUpdateAllScrollboxes();
}

function doClearParameterList(viewer = 0) {
    facefull.Lists["PL"+(viewer+1)].doClear();
    facefull.Scrollboxes['DLSB'+(viewer+1)].doUpdateScrollbar();
    doClearNeuronModel(viewer);
}

function doCreateParameterList(name, type = "neuron", viewer = -1) {
    console.log("creating param list for", name, type, viewer)
    if (viewer === -1) viewer = FManager.getCurrentFrameID();
    facefull.Lists["PL"+(viewer+1)].doClear();

    let hid = facefull.Lists["NMDL"].getState();
    let h = FManager.getCurrentFrame().getHistoryListItem(hid);
    console.log(h);

    if (type === "background") {
        doAddPanelParameter(name, "Structure name", FManager.getFrame(viewer).getData(h.structure_id).network_info.name, 0, 0, {type: type}, viewer);
        doAddPanelParameter(name, "Structure description", FManager.getFrame(viewer).getData(h.structure_id).network_info.desc, 0, 0, {type: type}, viewer);
        doAddPanelParameter(name, "Structure version", FManager.getFrame(viewer).getData(h.structure_id).network_info.version, 0, 0, {type: type}, viewer);
        doAddPanelParameter(name, "Parameter count", FManager.getFrame(viewer).getData(h.structure_id).network_info.parameter_count, 0, 0, {type: type}, viewer);
        doAddPanelParameter(name, "Model size", FManager.getFrame(viewer).getData(h.structure_id).network_info.model_size+" B", 0, 0, {type: type}, viewer);
        doClearNeuronModel(viewer);
        facefull.Scrollboxes['DLSB'+(viewer+1)].doUpdateScrollbar();
        return;
    }

    doAddPanelParameter(name, "Element name", name, 0, 0, {type: type});

    if (type !== "neuron") {
        doClearNeuronModel(viewer);
        facefull.Scrollboxes['DLSB'+(viewer+1)].doUpdateScrollbar();
        return;
    }

    ScopesEditList = [];
    PhantomEditList = [];

    let position_letters = ['x', 'y', 'z'];
    let spos = [];
    let rpos0 = [];
    let rpos = [];
    let rposf = [];

    // console.log("timeline data", FManager.getFrame(viewer).getData().timeline_data);

    let info = FManager.getFrame(viewer).getData(h.structure_id).neuron_list[name];
    let data = null;
    if (h.data_id >= 0 && FManager.getFrame(viewer).getData(h.structure_id).timeline_data && FManager.getFrame(viewer).getData(h.structure_id).timeline_data.length > h.data_id)
        data = FManager.getFrame(viewer).getData(h.structure_id).timeline_data[h.data_id].neurons[name];
    // console.log("parameter list", info, data);

    doAddPanelParameter(name, "Ensemble", info.ensemble);
    doAddPanelParameter(name, "Latency", info.latency!==undefined?info.latency:0);
    doAddPanelParameter(name, "Neuron size", info.size);
    doAddPanelParameter(name, "Dimensions", info.dimensions);
    doAddPanelParameter(name, "Inputs", info.input_signals);
    doAddPanelParameter(name, "Deviation", data&&data.deviation!==undefined?data.deviation:"N/A");
    doAddPanelParameter(name, "Synapses", "", 0, 1);

    for (let i = 0; i < info.synapses.length; i++) {
        if (info.synapses[i].type === "cluster") {
            let pos = doComputeClusterPosition(info.synapses[i].position, info.synapses[i].radius, info.input_signals.length, 4, "Synapse "+(i+1)+" (cluster)");
            spos = spos.concat(pos);

            doAddPanelParameter(name, "Synapse cluster", "", 1, 1, {id: i});
            doAddPanelParameter(name, "Position", info.synapses[i].position, 2, 0, {type: "synapse", id: i});
            doAddPanelParameter(name, "Radius",  info.synapses[i].radius, 2, 0, {type: "synapse", id: i});
            doAddPanelParameter(name, "Neurotransmitter type",  info.synapses[i].neurotransmitter, 2, 0, {type: "synapse", id: i});
            doAddPanelParameter(name, "k1",  info.synapses[i].k1, 2, 0, {type: "synapse", id: i});
        } else {
            spos.push({x: info.synapses[i].position[0], y: info.synapses[i].position[1], r: 4, type: "Synapse "+(i+1)});

            doAddPanelParameter(name, "Synapse", "", 1, 1,{id: i});
            doAddPanelParameter(name, "Entry", info.synapses[i].entry, 2, 0, {type: "synapse", id: i});
            doAddPanelParameter(name, "Position", info.synapses[i].position, 2, 0, {type: "synapse", id: i});
            // for (let p = 0; p < info.synapses[i].position.length; p++) {
            //     doAddPanelParameter(name, (p>3?"a"+(p-3):position_letters[p]), info.synapses[i].position[p], 3, 0);
            // }

            doAddPanelParameter(name, "Neurotransmitter type",  info.synapses[i].neurotransmitter, 2, 0, {type: "synapse", id: i});
            doAddPanelParameter(name, "k1",  info.synapses[i].k1, 2, 0 ,{type: "synapse", id: i});
        }
    }

    doAddPanelParameter(name, "Receptors", "", 0, 1);
    for (let i = 0; i < info.receptors.length; i++) {
        if (info.receptors[i].type === "cluster") {
            let pos = doComputeClusterPosition(info.receptors[i].position, info.receptors[i].radius, info.receptors[i].count, 2, "Receptor "+(i+1)+" (cluster)");
            rpos0 = rpos0.concat(pos);

            doAddPanelParameter(name, "Receptor cluster", "", 1, 1, {id: i});
            doAddPanelParameter(name, "Position", info.receptors[i].position, 2, 0, {type: "receptor", id: i});
            // for (let p = 0; p < info.receptors[i].position.length; p++) {
            //     doAddPanelParameter(name, (p>3?"a"+(p-3):position_letters[p]), info.receptors[i].position[p], 3, 0);
            // }
            doAddPanelParameter(name, "Count",  info.receptors[i].count, 2, 0, {type: "receptor", id: i});
            doAddPanelParameter(name, "Radius",  info.receptors[i].radius, 2, 0, {type: "receptor", id: i});
        } else {
            rpos0.push({x: info.receptors[i].position[0], y: info.receptors[i].position[1], r: 2, type: "Default receptor "+(i+1)});

            doAddPanelParameter(name, "Receptor", "", 1, 1, {id: i});
            doAddPanelParameter(name, "Position", info.receptors[i].position, 2, 0, {type: "receptor", id: i});
            // for (let p = 0; p < info.synapses[i].position.length; p++) {
            //     doAddPanelParameter(name, (p>3?"a"+(p-3):position_letters[p]), info.synapses[i].position[p], 3, 0);
            // }
            doAddPanelParameter(name, "Scopes", "", 2, 1, {type: "receptor", id: i});

            if (FManager.getCurrentFrameID() === FManager.getInterlinkFrameID() && data) {
                for (let j = 0; j < data.receptors[i].scopes.length; j++) {
                    doAddPanelParameter(name, "Scope "+j+" position", data.receptors[i].scopes[j], 3, 0, {type: "scope", id: i.toString()+"-"+j.toString()});
                    rpos.push({x: data.receptors[i].scopes[j][0],
                        y: data.receptors[i].scopes[j][1],
                        r: 2,
                        type: "Reference receptor "+(i+1)+" (scope "+j+")"});
                }
                if (data.receptors[i].phantom) {
                    doAddPanelParameter(name, "Phantom position", data.receptors[i].phantom, 2, 1, {type: "phantom", id: i});
                    rposf.push({x: data.receptors[i].phantom[0],
                        y: data.receptors[i].phantom[1],
                        r: 4,
                        type: "Phantom receptor "+(i+1)});
                }
            }
        }
    }

    facefull.Scrollboxes['DLSB'+(viewer+1)].doUpdateScrollbar();
    doShowNeuronModel(info.size, spos, rpos0, rpos, rposf);
}

function doAddPanelParameter(name, label, value, level = 0, arrow = 0, info = {}, viewer = -1) {
    if (viewer === -1) viewer = FManager.getCurrentFrameID();
    let catlabels = ["Synapses", "Receptors"];
    let ellabels = ["Synapse", "Synapse cluster", "Receptor", "Receptor cluster"];

    let eedit = document.createElement("label");
    if (viewer === FManager.getInterlinkFrameID())
        eedit.innerHTML = "<input type='text' class='Edit' disabled>";
    else
        eedit.innerHTML = "<input type='text' class='Edit'>";

    eedit.children[0].value = value;

    if (info.type === "scope") {
        ScopesEditList.push({element: eedit.children[0]});
    }
    if (info.type === "phantom") {
        PhantomEditList.push({element: eedit.children[0]});
    }
    if (arrow === 0) {
        facefull.Lists["PL"+(viewer+1)].doAdd([label, {element: eedit}], level);
    } else {
        facefull.Lists["PL"+(viewer+1)].doAdd([label, {element: eedit}], level, {action: "arrow"});
    }
}
