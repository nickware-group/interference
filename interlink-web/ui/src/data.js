// let Data = [];
// let FManager.getFrame(viewer).getData().entry_list = [];
// let FManager.getFrame(viewer).getData().neuron_list = {};
// let FManager.getFrame(viewer).getData().output_list = [];
// let FManager.getFrame(viewer).getData().network_info = {};
// let CurrentSelectedNeuron = "";

const ENTRY_POS_Y_SHIFT = 100;
let ElementsForCopy = [];
let SourceForCopy = 0;
let PastePos = {x: 0, y: 0};
// let ActionHistory = [];

facefull.doEventHandlerAttach("doProcessData", function(str) {
    if (str === "") return;

    doProcessData(0, str, false);
    doRefreshProjectStats();
    setProjectChanged();
});

facefull.doEventHandlerAttach("doProcessInterlinkData", function(str) {
    doProcessData(FManager.getInterlinkFrameID(), str, false);
    doRefreshInterlinkStats();
});

facefull.doEventHandlerAttach("doUpdateStructure", function(str) {
    doProcessData(FManager.getInterlinkFrameID(), str, false);
    doRefreshInterlinkStats();
});

facefull.doEventHandlerAttach("doUpdateData", function(str) {
    if (str === "") return;
    try {
        let data = JSON.parse(str);
        for (let n in data.neurons) {
            let name = data.neurons[n].name;
            doHighlightNode(name);

            // console.log("update data for", name);
            // console.log(FManager.getInterlinkFrame().getData().neuron_list[name].receptors)
            // console.log(data.neurons[n].receptors)

            if (data.neurons[n].receptors)
                for (let i = 0; i < data.neurons[n].receptors.length && i < FManager.getInterlinkFrame().getData().neuron_list[name].receptors.length; i++) {
                    FManager.getInterlinkFrame().getData().neuron_list[name].receptors[i].data_scopes.push({
                        scopes: data.neurons[n].receptors[i].scopes,
                        phantom: data.neurons[n].receptors[i].phantom
                    })
                }

            let current = FManager.getInterlinkFrame().getLastSelectedElement();

            // console.log("update data for current", current)

            if (current === name) {
                console.log(data);

                let spos = [];
                let rpos0 = [];
                let rpos = [];
                let rposf = [];

                for (let i = 0; i < FManager.getInterlinkFrame().getData().neuron_list[current].synapses.length; i++) {
                    spos.push({x: FManager.getInterlinkFrame().getData().neuron_list[current].synapses[i].position[0],
                        y: FManager.getInterlinkFrame().getData().neuron_list[current].synapses[i].position[1],
                        r: 4,
                        type: "Synapse "+(i+1)});
                }

                for (let i = 0; i < FManager.getInterlinkFrame().getData().neuron_list[current].receptors.length; i++) {
                    rpos0.push({x: FManager.getInterlinkFrame().getData().neuron_list[current].receptors[i].position[0],
                        y: FManager.getInterlinkFrame().getData().neuron_list[current].receptors[i].position[1],
                        r: 2,
                        type: "Default receptor "+(i+1)});
                }

                console.log("phantom", PhantomEditList);
                let current_data_scope = facefull.Lists["NMDL"].getState();
                let local_neuron_data = FManager.getInterlinkFrame().getData().neuron_list[name];

                if (!current_data_scope) current_data_scope = 0;

                for (let i = 0; i < local_neuron_data.receptors.length; i++) {
                    let local_data_scope = current_data_scope;
                    if (local_data_scope >= local_neuron_data.receptors[i].data_scopes.length) local_data_scope = local_neuron_data.receptors[i].data_scopes.length-1;

                    for (let j = 0; j < local_neuron_data.receptors[i].data_scopes[local_data_scope].scopes.length; j++) {
                        if (ScopesEditList[i*local_neuron_data.receptors.length+j])
                            ScopesEditList[i*local_neuron_data.receptors.length+j].element.value = local_neuron_data.receptors[i].data_scopes[local_data_scope].scopes[j];
                        rpos.push({x: local_neuron_data.receptors[i].data_scopes[local_data_scope].scopes[j][0],
                            y: local_neuron_data.receptors[i].data_scopes[local_data_scope].scopes[j][1],
                            r: 2,
                            type: "Reference receptor "+(i+1)+" (scope "+j+")"});
                    }
                    if (PhantomEditList[i])
                        PhantomEditList[i].element.value = local_neuron_data.receptors[i].data_scopes[local_data_scope].phantom;
                    rposf.push({x: local_neuron_data.receptors[i].data_scopes[local_data_scope].phantom[0],
                        y: local_neuron_data.receptors[i].data_scopes[local_data_scope].phantom[1],
                        r: 4,
                        type: "Phantom receptor "+(i+1)});
                }

                doShowNeuronModel(FManager.getInterlinkFrame().getData().neuron_list[name].size, spos, rpos0, rpos, rposf, FManager.getInterlinkFrameID());
                // doShowMetrics(facefull.Comboboxes["MRCB"].setState(facefull.Comboboxes["MRCB"].getState()));
            }
        }
    } catch (e) {
        console.log(e);
    }
});

facefull.doEventHandlerAttach("doUpdateMetrics", function(str) {
    let data = JSON.parse(str);
    doShowMetrics(data);
});

facefull.doEventHandlerAttach("doAddMetricsToList", function(str) {
    doAddMetricsToList(str);
});

facefull.doEventHandlerAttach("onExportData", function(str) {
    AlertShow("Export successful", "Neural network structure exported successfully.", "info");

});

function doProcessData(viewer, str, full_update = true) {
    if (FManager.getFrame(viewer).getString() === str) return;
    AlertShow("Loading", "Loading neural network structure...", "info", "");
    try {
        let elements = [];
        let enpos = 0;

        let data = JSON.parse(str);
        let ne_list = {};

        console.log("done", performance.now(), data);
        FManager.getFrame(viewer).doClearLists();
        FManager.getFrame(viewer).doUpdateString(str);

        for (let i = 0; i < data.entries.length; i++) {
            elements.push({
                group: 'nodes',
                classes: 'viewer-node-small',
                data: { id: data.entries[i], name: data.entries[i], weight: 1  },
                selectable: true,
                grabbable: true
                //renderedPosition: {x: 0, y: enpos}
            });

            enpos += ENTRY_POS_Y_SHIFT;
            FManager.getFrame(viewer).getData().entry_list.push(data.entries[i]);
            facefull.Lists["IL"+(viewer+1)].doAdd([data.entries[i]]);
        }

        let ensemble = "";
        for (let i = 0; i < data.neurons.length; i++) {
            elements.push({
                group: 'nodes',
                data: { id: data.neurons[i].name, name: data.neurons[i].name, weight: -1 },
                selectable: true,
                grabbable: true
            });

            if (!data.neurons[i].synapses) data.neurons[i].synapses = [];
            if (!data.neurons[i].receptors) data.neurons[i].receptors = [];

            FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name] = data.neurons[i];
            for (let r = 0; r < FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name].receptors.length; r++) {
                FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name].receptors[r].data_scopes = [];
            }

            if (data.neurons[i].ensemble === undefined) {
                data.neurons[i].ensemble = "";
            }
            let ensemble = data.neurons[i].ensemble;
            if (!ne_list[ensemble]) ne_list[ensemble] = [];
            ne_list[ensemble].push(data.neurons[i].name);

            console.log(data.neurons[i].ensemble)

            for (let j = 0; data.neurons[i].input_signals && j < data.neurons[i].input_signals.length; j++) {
                let found1 = data.entries.find((value) => data.neurons[i].input_signals[j] === value);
                let found2 = data.neurons.find((value) => data.neurons[i].input_signals[j] === value.name);

                if (found1 || found2) {
                    elements.push({
                        data: { id: data.neurons[i].input_signals[j]+"-"+data.neurons[i].name, source: data.neurons[i].input_signals[j], target: data.neurons[i].name },
                    });
                }
            }
        }

        for (let e in ne_list) {
            let en_name = e;
            if (en_name === "") en_name = "No ensemble";
            facefull.Lists["EL"+(viewer+1)].doAdd([en_name], 0, {action: "arrow"});
            for (let n in ne_list[e]) {
                facefull.Lists["EL"+(viewer+1)].doAdd([ne_list[e][n]], 1);
            }
        }

        for (let i = 0; data.output_signals && i < data.output_signals.length; i++) {
            elements.push({
                group: 'nodes',
                classes: 'viewer-node-small',
                data: { id: "O"+(i+1), name: "Output "+(i+1), weight: 2 },
                selectable: true,
                grabbable: true
            });

            elements.push({
                data: { id: "o"+i, source: data.output_signals[i], target: "O"+(i+1) }
            });
            FManager.getFrame(viewer).getData().output_list.push({id: "O"+(i+1), link: data.output_signals[i]});
            facefull.Lists["OL"+(viewer+1)].doAdd(["Output "+(i+1)]);
        }

        FManager.getFrame(viewer).getData().network_info.name = data.name;
        FManager.getFrame(viewer).getData().network_info.desc = data.desc;
        FManager.getFrame(viewer).getData().network_info.version = data.version;

        facefull.Scrollboxes["ELSB"+(viewer+1)].doUpdateScrollbar();
        facefull.Scrollboxes["ILSB"+(viewer+1)].doUpdateScrollbar();
        facefull.Scrollboxes["OLSB"+(viewer+1)].doUpdateScrollbar();

        console.log("done", performance.now());
        console.log("process", viewer,FManager.getFrame(viewer));

        //if (full_update) FManager.getFrame(viewer).doInitViewer(elements, FManager.getFrame(viewer).getData().entry_list[0]);
        //else
        FManager.getFrame(viewer).doUpdateViewer(elements);
        doCreateParameterList("", "background", viewer);
    } catch (e) {
        console.log(e);
    }
    AlertHideCustom("AE");
}

function doRefreshNeuronList(viewer) {
    facefull.Lists["EL"+(viewer+1)].doClear();

    let ne_list = {};
    for (let n in FManager.getFrame(viewer).getData().neuron_list) {
        let ensemble = FManager.getFrame(viewer).getData().neuron_list[n].ensemble;
        if (!ne_list[ensemble]) ne_list[ensemble] = [];
        ne_list[ensemble].push(FManager.getFrame(viewer).getData().neuron_list[n].name);
    }

    for (let e in ne_list) {
        let en_name = e;
        if (en_name === "") en_name = "No ensemble";
        facefull.Lists["EL"+(viewer+1)].doAdd([en_name], 0, {action: "arrow"});
        for (let n in ne_list[e]) {
            facefull.Lists["EL"+(viewer+1)].doAdd([ne_list[e][n]], 1);
        }
    }

    facefull.Scrollboxes["ELSB"+(viewer+1)].doUpdateScrollbar();
    console.log("refreshed neuron list")
    doRefreshProjectStats();
}

function doRefreshEntryList(viewer) {
    facefull.Lists["IL"+(viewer+1)].doClear();
    for (let e in FManager.getFrame(viewer).getData().entry_list) {
        facefull.Lists["IL"+(viewer+1)].doAdd([FManager.getFrame(viewer).getData().entry_list[e]]);
    }
    facefull.Scrollboxes["ILSB"+(viewer+1)].doUpdateScrollbar();
    doRefreshProjectStats();
}

function doRefreshOutputList(viewer) {
    facefull.Lists["OL"+(viewer+1)].doClear();
    let nodes = FManager.getFrame(viewer).getViewer().cs.filter('node[weight = 2]');
    for (let i = 0; i < nodes.length; i++) {
        facefull.Lists["OL"+(viewer+1)].doAdd(["Output "+(i+1)]);
    }
    facefull.Scrollboxes["OLSB"+(viewer+1)].doUpdateScrollbar();
    doRefreshProjectStats();
}

function doRefreshData(viewer, data) {
    console.log("refresh data", data, viewer);
    console.log("refresh data", FManager.getFrame(viewer));

    FManager.getFrame(viewer).doClearLists();

    FManager.getFrame(viewer).getData().entry_list = data.entry_list;
    FManager.getFrame(viewer).getData().neuron_list = data.neuron_list;
    FManager.getFrame(viewer).getData().output_list = data.output_list;
    FManager.getFrame(viewer).getData().network_info = data.network_info;

    FManager.getFrame(viewer).doImportViewer(data.viewer);

    doCreateParameterList("", "background", viewer);

    doRefreshEntryList(viewer);
    doRefreshNeuronList(viewer);
    doRefreshOutputList(viewer);
}

function doDeleteSynapse(id) {
    FManager.getCurrentFrame().getData().neuron_list[FManager.getCurrentFrame().getLastSelectedElement()].synapses.splice(id, 1);

    doClearParameterList();
    doCreateParameterList(FManager.getCurrentFrame().getLastSelectedElement());
    facefull.Lists["PL1"].elist.children[6].children[0].click();
    setProjectChanged();
}

function doDeleteReceptor(id) {
    FManager.getCurrentFrame().getData().neuron_list[FManager.getCurrentFrame().getLastSelectedElement()].receptors.splice(id, 1);

    doClearParameterList();
    doCreateParameterList(FManager.getCurrentFrame().getLastSelectedElement());
    for (let i = 7; i < facefull.Lists["PL1"].elist.children.length; i++) {
        if (facefull.Lists["PL1"].elist.children[i].children[0].innerHTML === "Receptors") {
            facefull.Lists["PL1"].elist.children[i].children[0].click();
            break;
        }
    }
    setProjectChanged();
}

function doAddSynapse(type) {
    FManager.getCurrentFrame().getData().neuron_list[FManager.getCurrentFrame().getLastSelectedElement()].synapses.push({
        type: type,
        position: [0,0,0],
        radius: 10,
        neurotransmitter: "activation",
        k1: 1,
        entry: 0
    });

    doCreateParameterList(FManager.getCurrentFrame().getLastSelectedElement());
    facefull.Lists["PL1"].elist.children[6].children[0].click();
    setProjectChanged();
}

function doAddReceptor(type) {
    FManager.getCurrentFrame().getData().neuron_list[FManager.getCurrentFrame().getLastSelectedElement()].receptors.push({
        type: type,
        position: [0,0,0],
        radius: 10,
        count: 10
    });

    doCreateParameterList(FManager.getCurrentFrame().getLastSelectedElement());
    for (let i = 7; i < facefull.Lists["PL1"].elist.children.length; i++) {
        if (facefull.Lists["PL1"].elist.children[i].children[0].innerHTML === "Receptors") {
            facefull.Lists["PL1"].elist.children[i].children[0].click();
            break;
        }
    }
    setProjectChanged();
}

function doAddNode(pos, name = "", type = 0) {
    let newname = name;
    if (!type) {
        let keys = Object.keys(FManager.getCurrentFrame().getData().neuron_list);

        if (name === "") {
            let num = 0;
            for (let k in keys) {
                let key = keys[k];
                if (key.charAt(0) === 'N' && !isNaN(parseInt(key.substring(1)))) {
                    let newnum = parseInt(key.substring(1));
                    if (num < newnum) {
                        num = newnum;
                    }
                }
            }
            newname = 'N'+(num+1);
        }

        FManager.getCurrentFrame().getData().neuron_list[newname] = {
            name: newname,
            ensemble: "",
            latency: 0,
            dimensions: 2,
            size: 100,
            input_signals: [],
            synapses: [],
            receptors: [],
        };
        doRefreshNeuronList(FManager.getCurrentFrameID());
        doRenderNewNode(newname, pos, type);
    } else if (type === 1) {
        if (name === "") {
            let num = 0;
            for (let en in FManager.getCurrentFrame().getData().entry_list) {
                let key = FManager.getCurrentFrame().getData().entry_list[en];
                if (key.charAt(0) === 'E' && !isNaN(parseInt(key.substring(1)))) {
                    let newnum = parseInt(key.substring(1));
                    if (num < newnum) {
                        num = newnum;
                    }
                }
            }
            newname = 'E'+(num+1);
        }

        FManager.getCurrentFrame().getData().entry_list.push(newname);
        facefull.Lists["IL"+(FManager.getCurrentFrameID()+1)].doAdd([newname]);
        doRenderNewNode(newname, pos, type);
    } else if (type === 2) {
        let nodes = FManager.getCurrentFrame().getData().output_list;
        let num = nodes.length;
        let lastid = 0;
        if (num)
            lastid = parseInt(nodes[num-1].id.substring(1));
        newname = 'O'+(lastid+1);
        // console.log(nodes, num);
        facefull.Lists["OL"+(FManager.getCurrentFrameID()+1)].doAdd(["Output "+(num+1)]);
        FManager.getCurrentFrame().getData().output_list.push({id: 'O'+(lastid+1), link: ""});
        doRenderNewNode(newname, pos, type, "Output "+(num+1));
    }
    console.log("add node", newname, type);
    setProjectChanged();
    return newname;
}

function doUpdateInputs(viewer, current, str) {
    let edges = FManager.getFrame(viewer).getViewer().cs.edges('[target = "'+current+'"]');
    for (let i = 0; i < edges.length; i++) {
        edges[i].remove();
    }

    if (str === "") {
        FManager.getCurrentFrame().getData().neuron_list[current].input_signals = [];
        return;
    }

    let inputs = str.split(',');
    console.log("doUpdateInputs", edges, inputs.length);
    // for (let e in edges) {
    //     e.remove();
    // }

    for (let i = 0; i < inputs.length; i++) {
        // let found = edges.findIndex(function(e) {
        //     return e.data("source") === inputs[i];
        // });
        // console.log(inputs[i], found);
        try {
            doAddLink(inputs[i].trim(), current);
        } catch (e) {
            console.log("Error", e);
        }
    }
    FManager.getCurrentFrame().getData().neuron_list[current].input_signals = inputs;
}

function doComputeClusterPosition(pos, radius, count, r, type="Cluster") {
    let x = parseInt(pos[0]);
    let y = parseInt(pos[1]);
    let npos = [];
    radius = parseInt(radius);

    let dfi = 360. / count;
    let fi = 0;
    let xr, yr;
    for (let i = 0; i < count; i++) {
        xr = x + radius * Math.cos(fi/180*Math.PI);
        yr = y + radius * Math.sin(fi/180*Math.PI);
        npos.push({x: xr, y: yr, r: r, type: type});
        fi += dfi;
    }
    return npos;
}

function doUpdateName(viewer, name, nname, type) {
    if (name === nname) return;

    let keys = Object.keys(FManager.getFrame(viewer).getData().neuron_list);
    let found = keys.findIndex(function(e) {
        return e === nname;
    });

    if (found !== -1 && FManager.getFrame(viewer).getData().neuron_list[nname] !== null) {
        console.log("Found", FManager.getFrame(viewer).getData().neuron_list[nname]);
        AlertShow("Error", "Element already exists.", "error");
        doCreateParameterList(name);
        return;
    }

    let node = FManager.getCurrentFrame().getViewer().cs.filter('node[id = "'+name+'"]');
    let pos = node.renderedPosition();
    node.remove();

    if (type === "neuron") {
        doRenderNewNode(nname, pos, 0);
        FManager.getCurrentFrame().getData().neuron_list[nname] = FManager.getCurrentFrame().getData().neuron_list[name];
        FManager.getCurrentFrame().getData().neuron_list[nname].name = nname;
        delete FManager.getCurrentFrame().getData().neuron_list[name];

        for (let i in FManager.getCurrentFrame().getData().neuron_list[nname].input_signals) {
            doAddLink(FManager.getCurrentFrame().getData().neuron_list[nname].input_signals[i], nname);
        }

        for (let n in FManager.getCurrentFrame().getData().neuron_list) {
            if (FManager.getCurrentFrame().getData().neuron_list[n] !== null) {
                for (let i in FManager.getCurrentFrame().getData().neuron_list[n].input_signals) {
                    if (FManager.getCurrentFrame().getData().neuron_list[n].input_signals[i] === name) {
                        FManager.getCurrentFrame().getData().neuron_list[n].input_signals[i] = nname;
                        doAddLink(nname, FManager.getCurrentFrame().getData().neuron_list[n].name);
                    }
                }
            }
        }

        for (let o in FManager.getCurrentFrame().getData().output_list) {
            if (FManager.getCurrentFrame().getData().output_list[o] === name) {
                FManager.getCurrentFrame().getData().output_list[o] = nname;
                doAddLink(nname, "O"+(parseInt(o)+1));
            }
        }
        doRefreshNeuronList(viewer);
    } else if (type === "entry") {
        doRenderNewNode(nname, pos, 1);
        for (let e in FManager.getCurrentFrame().getData().entry_list) {
            if (FManager.getCurrentFrame().getData().entry_list[e] === name) {
                FManager.getCurrentFrame().getData().entry_list[e] = nname;
                break;
            }
        }
        for (let n in FManager.getCurrentFrame().getData().neuron_list) {
            for (let i in FManager.getCurrentFrame().getData().neuron_list[n].input_signals) {
                if (FManager.getCurrentFrame().getData().neuron_list[n].input_signals[i] === name) {
                    FManager.getCurrentFrame().getData().neuron_list[n].input_signals[i] = nname;
                    doAddLink(nname, FManager.getCurrentFrame().getData().neuron_list[n].name);
                }
            }
        }
        doRefreshEntryList(viewer);
    }
    // else if (type === "output") {
    //     let oid = parseInt(nname.substr(1));
    //     doRenderNewNode(nname, pos, 2, "Output "+oid);
    //     //doAddLink(, nname);
    // }
    doCreateParameterList(nname);
}


function doCheckNeuronForCopy(name, source = 0) {
    ElementsForCopy = [...name];
    SourceForCopy = source;
    console.log("set for copy", ElementsForCopy, SourceForCopy);
}

function doPasteElement() {
    console.log("pasting", ElementsForCopy, PastePos, SourceForCopy);
    let inputs_calibration = [];
    let reference_pos = {};
    let current_pos = {};
    if (ElementsForCopy.length > 0) {
        for (let i in ElementsForCopy) {
            let nname = ElementsForCopy[i];
            let source_obj = FManager.getFrame(SourceForCopy).getViewer().cs.filter('node[id = "'+ElementsForCopy[i]+'"]');
            if (i === "0") {
                reference_pos = source_obj.renderedPosition();
            }
            current_pos = source_obj.renderedPosition();
            console.log("dbg", source_obj.data('weight'), reference_pos, current_pos)

            if (reference_pos.x > current_pos.x) current_pos.x = PastePos.x + (reference_pos.x - current_pos.x);
            else current_pos.x = PastePos.x + (current_pos.x - reference_pos.x);

            if (reference_pos.y > current_pos.y) current_pos.y = PastePos.y + (reference_pos.y - current_pos.y);
            else current_pos.y = PastePos.y + (current_pos.y - reference_pos.y);

            switch (source_obj.data('weight')) {
                case -1:
                    for (let e = 2;; e++) {
                        let found = false;
                        for (let n in FManager.getCurrentFrame().getData().neuron_list) {
                            if (FManager.getCurrentFrame().getData().neuron_list[n].name === nname) {
                                nname = ElementsForCopy[i]+"("+e+")";
                                found = true;
                                break;
                            }
                        }
                        if (!found) break;
                    }
                    doAddNode(current_pos, nname, 0);
                    FManager.getCurrentFrame().getData().neuron_list[nname] = JSON.parse(JSON.stringify(FManager.getFrame(SourceForCopy).getData().neuron_list[ElementsForCopy[i]]));
                    FManager.getCurrentFrame().getData().neuron_list[nname].name = nname;
                    inputs_calibration.push({before: ElementsForCopy[i], after: nname, flag: 0});
                    doRefreshNeuronList(FManager.getCurrentFrameID());

                    break;
                case 1:
                    for (let e = 3;; e++) {
                        let found = false;
                        for (let n in FManager.getCurrentFrame().getData().entry_list) {
                            if (FManager.getCurrentFrame().getData().entry_list[n] === nname) {
                                nname = ElementsForCopy[i]+"("+e+")";
                                found = true;
                                break;
                            }
                        }
                        if (!found) break;
                    }
                    doAddNode(current_pos, nname, 1);
                    inputs_calibration.push({before: ElementsForCopy[i], after: nname, flag: 1});

                    break;
                case 2:
                    let node_name = doAddNode(current_pos, "", 2);
                    let olist = FManager.getFrame(SourceForCopy).getViewer().cs.edges('[target = "'+ElementsForCopy[i]+'"]');
                    console.log("pasting output", ElementsForCopy[i], olist, olist.length, olist[0].data("source"));
                    if (olist.length) {
                        inputs_calibration.push({before: node_name, after: olist[0].data("source"), flag: 2});
                    }
                    break;
            }
        }

        for (let i in inputs_calibration) {
            if (inputs_calibration[i].flag === 0) {
                let inputs = [];
                for (let is in FManager.getCurrentFrame().getData().neuron_list[inputs_calibration[i].after].input_signals) {
                    let input_name = FManager.getCurrentFrame().getData().neuron_list[inputs_calibration[i].after].input_signals[is];
                    let iitem = inputs_calibration.findIndex(function(element) {
                        return element.before === input_name;
                    });
                    if (iitem >= 0) {
                        inputs.push(inputs_calibration[iitem].after);
                    }
                }
                // console.log("adding inputs", inputs);
                doUpdateInputs(FManager.getCurrentFrameID(), inputs_calibration[i].after, inputs.toString());
            } else if (inputs_calibration[i].flag === 2) {
                console.log("output link calib", inputs_calibration[i])
                let iitem = inputs_calibration.findIndex(function(element) {
                    return element.before === inputs_calibration[i].after;
                });
                if (iitem >= 0) {
                    doAddLink(inputs_calibration[iitem].after, inputs_calibration[i].before)
                }
            }
        }

        PastePos.x += 30;
        PastePos.y += 30;
    }
}


function doRemoveElement(e) {
    let list = [...e];
    for (let i in list) {
        let name = list[i];
        FManager.getCurrentFrame().doRemoveSelectedElement(name);
        let node = FManager.getCurrentFrame().getViewer().cs.filter('node[id = "'+name+'"]');
        let type = node.data('weight');
        let ename = node.data('name');
        node.remove();
        switch (type) {
            case -1:
                delete FManager.getCurrentFrame().getData().neuron_list[name];
                doRefreshNeuronList(FManager.getCurrentFrameID());
                break;
            case 1:
                let eitem = FManager.getCurrentFrame().getData().entry_list.indexOf(name);
                if (eitem >= 0) {
                    FManager.getCurrentFrame().getData().entry_list.splice(eitem, 1);
                    doRefreshEntryList(FManager.getCurrentFrameID());
                }
                break;
            case 2:
                let oitem = FManager.getCurrentFrame().getData().output_list.findIndex(function(el) {
                    return el.id === name;
                });
                //let oitem = parseInt(name.substr(1)) - 1;

                if (oitem >= 0) {
                    console.log("deleting output", name,  ename, oitem, FManager.getCurrentFrame().getData().output_list)
                    FManager.getCurrentFrame().getData().output_list.splice(oitem, 1);
                    doRefreshOutputList(FManager.getCurrentFrameID());

                    // let onodes = FManager.getCurrentFrame().getViewer().cs.filter('node[weight = 2]');

                    for (let i in FManager.getCurrentFrame().getData().output_list) {
                        let oid = FManager.getCurrentFrame().getData().output_list[i].id;
                        let onode = FManager.getCurrentFrame().getViewer().cs.filter('node[id = "'+oid+'"]');
                        onode.data("name", "Output "+(parseInt(i)+1));
                    }
                    // let id = 0;
                    // for (let i in onodes) {
                    //     id++;
                    //     console.log("enum outputs", onodes[i].id());
                    //     onodes[i].data("name", "Output "+id);
                    //     //doUpdateName(FManager.getCurrentFrameID(), onodes[i].data("id"), "O"+id, "output");
                    // }
                }
                break;
        }
    }
}

function doUpdateValues(name, event, label, info) {
    let value = event.target.value;
    console.log(name, label, value, info);

    switch (label) {
        case "Inputs":
            doUpdateInputs(FManager.getCurrentFrameID(), name, value);
            doUpdateModel(FManager.getCurrentFrameID(), name);
            doRefreshProjectStats();
            break;

        case "Element name":
            doUpdateName(FManager.getCurrentFrameID(), name, value, info.type);
            break;

        case "Ensemble":
            FManager.getCurrentFrame().getData().neuron_list[name].ensemble = value;
            doRefreshNeuronList(0);
            break;

        case "Latency":
            FManager.getCurrentFrame().getData().neuron_list[name].latency = parseInt(value);
            break;

        case "Neuron size":
            FManager.getCurrentFrame().getData().neuron_list[name].size = parseInt(value);
            doUpdateModel(FManager.getCurrentFrameID(), name);
            break;

        case "Dimensions":
            FManager.getCurrentFrame().getData().neuron_list[name].dimensions = parseInt(value);
            break;

        case "Position":
            let pos = value.split(',');
            //console.log(value.split(','))
            for (let p in pos) {
                pos[p] = parseInt(pos[p]);
            }

            if (info.type === "synapse")
                FManager.getCurrentFrame().getData().neuron_list[name].synapses[info.id].position = pos;
            else
                FManager.getCurrentFrame().getData().neuron_list[name].receptors[info.id].position = pos;
            doUpdateModel(FManager.getCurrentFrameID(), name);
            break;

        case "Radius":
            if (info.type === "synapse")
                FManager.getCurrentFrame().getData().neuron_list[name].synapses[info.id].radius = parseInt(value);
            else
                FManager.getCurrentFrame().getData().neuron_list[name].receptors[info.id].radius = parseInt(value);
            doUpdateModel(FManager.getCurrentFrameID(), name);
            break;

        case "k1":
            FManager.getCurrentFrame().getData().neuron_list[name].synapses[info.id].k1 = parseInt(value);
            break;

        case "Entry":
            FManager.getCurrentFrame().getData().neuron_list[name].synapses[info.id].entry = parseInt(value);
            break;

        case "Neurotransmitter type":
            FManager.getCurrentFrame().getData().neuron_list[name].synapses[info.id].neurotransmitter = value;
            break;

        case "Count":
            FManager.getCurrentFrame().getData().neuron_list[name].receptors[info.id].count = parseInt(value);
            doUpdateModel(FManager.getCurrentFrameID(), name);
            break;

        case "Structure name":
            FManager.getCurrentFrame().getData().network_info.name = value;
            break;

        case "Structure description":
            FManager.getCurrentFrame().getData().network_info.desc = value;
            break;

        case "Structure version":
            FManager.getCurrentFrame().getData().network_info.version = value;
            break;
    }

    setProjectChanged();
}

function getStructure(viewer = 0) {
    let data = {};
    data.entries = FManager.getFrame(viewer).getData().entry_list;

    let neurons = [];
    for (let items in FManager.getFrame(viewer).getData().neuron_list) {
        neurons.push(FManager.getFrame(viewer).getData().neuron_list[items]);
    }
    data.neurons = neurons;

    data.output_signals = [];
    for (let i in FManager.getFrame(viewer).getData().output_list) {
        let link = FManager.getFrame(viewer).getData().output_list[i].link;
        if (link !== "")
            data.output_signals.push(link);
    }

    data.name = FManager.getFrame(viewer).getData().network_info.name;
    data.desc = FManager.getFrame(viewer).getData().network_info.desc;
    data.version = FManager.getFrame(viewer).getData().network_info.version;
    return data;
}

function getStructureSize(viewer = 0) {
    let size = 0;
    let int_size = 4;
    let float_size = 4;

    for (let items in FManager.getFrame(viewer).getData().neuron_list) {
        let neuron = FManager.getFrame(viewer).getData().neuron_list[items];

        size += neuron.name.length;
        size += int_size; // for neuron size
        size += int_size; // for neuron dimensions

        for (let i in neuron.input_signals) {
            size += neuron.input_signals[i].length;
        }

        for (let i in neuron.output_signals) {
            size += neuron.output_signals[i].length;
        }

        for (let i in neuron.receptors) {
            let object_size = neuron.receptors[i].position.length * float_size;
            object_size *= 3; // for default, reference scope and phantom position

            if (neuron.receptors[i].type && neuron.receptors[i].type === "cluster") {
                object_size *= neuron.receptors[i].count;
            }

            size += object_size;
        }

        for (let i in neuron.synapses) {
            let object_size = neuron.synapses[i].position.length * float_size;
            object_size += int_size;    // for nt type
            object_size += float_size;  // for k1
            object_size += float_size;  // for k2
            object_size += float_size;  // for Lambda
            object_size += float_size;  // for Gamma

            if (neuron.synapses[i].type && neuron.synapses[i].type === "cluster") {
                object_size *= neuron.input_signals.length;
            }

            size += object_size;
        }
    }

    return size;
}
