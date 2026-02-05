const ENTRY_POS_Y_SHIFT = 100;
let ElementsForCopy = [];
let SourceForCopy = 0;
let PastePos = {x: 0, y: 0};

facefull.doEventHandlerAttach("doProcessData", function(str) {
    if (str === "") return;

    doProcessData(0, str, false);
});

facefull.doEventHandlerAttach("doProcessInterlinkData", function(str) {
    doProcessData(FManager.getInterlinkFrameID(), str, false);
});

facefull.doEventHandlerAttach("doUpdateStructure", function(str) {
    doProcessData(FManager.getInterlinkFrameID(), str, false);
});

facefull.doEventHandlerAttach("doUpdateData", function(str) {
    if (str === "") return;
    try {
        let data_batches = JSON.parse(str);
        setInterlinkConnectionStatus(true);

        for (let d in data_batches) {
            let data = JSON.parse(data_batches[d]);

            FManager.getInterlinkFrame().getData().timeline_data.push(data);

            for (let n in data.neurons) {
                let neuron = data.neurons[n];
                let name = neuron.name;
                doHighlightNode(name);

                // console.log("update data for", name);
                // console.log(FManager.getInterlinkFrame().getData().neuron_list[name].receptors)
                // console.log(data.neurons[n].receptors)

                let current = FManager.getInterlinkFrame().getLastSelectedElement();

                // console.log("update data for current", current)

                if (current === name) {
                    console.log("current neuron data", neuron);

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

                    for (let i = 0; i < neuron.receptors.length; i++) {
                        // let local_data_scope = current_data_scope;
                        // if (local_data_scope >= local_neuron_data.receptors[i].data_scopes.length) {
                        //     local_data_scope = local_neuron_data.receptors[i].data_scopes.length-1;
                        // }

                        if (neuron.receptors[i].scopes !== undefined) {
                            for (let j = 0; j < neuron.receptors[i].scopes.length; j++) {
                                if (ScopesEditList[i*neuron.receptors.length+j])
                                    ScopesEditList[i*neuron.receptors.length+j].element.value = neuron.receptors[i].scopes[j];
                                rpos.push({x: neuron.receptors[i].scopes[j][0],
                                    y: neuron.receptors[i].scopes[j][1],
                                    r: 2,
                                    type: "Reference receptor "+(i+1)+" (scope "+j+")"});
                            }
                        }

                        if (neuron.receptors[i].phantom !== undefined) {
                            if (PhantomEditList[i])
                                PhantomEditList[i].element.value = neuron.receptors[i].phantom;
                            rposf.push({x: neuron.receptors[i].phantom[0],
                                y: neuron.receptors[i].phantom[1],
                                r: 4,
                                type: "Phantom receptor "+(i+1)});
                        }
                    }

                    doShowNeuronModel(FManager.getInterlinkFrame().getData().neuron_list[name].size, spos, rpos0, rpos, rposf, FManager.getInterlinkFrameID());
                    // doShowMetrics(facefull.Comboboxes["MRCB"].setState(facefull.Comboboxes["MRCB"].getState()));
                }
            }
            FManager.getInterlinkFrame().doAddModelHistoryToList("Data update");
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

function doProcessData(viewer, str, full_update = true) {
    if (FManager.getFrame(viewer).getString() === str) return;
    setInterlinkConnectionStatus(true);

    try {
        let data = JSON.parse(str);

        FManager.getFrame(viewer).doCreateModel();

        console.log("done", performance.now(), data);
        FManager.getFrame(viewer).getViewer().clear();
        FManager.getFrame(viewer).doClearLists();
        FManager.getFrame(viewer).doUpdateString(str);

        for (let i = 0; i < data.entries.length; i++) {
            doRenderNewNode(data.entries[i], null, 1, data.entries[i]);
            FManager.getFrame(viewer).getData().entry_list.push(data.entries[i]);
        }

        let ensemble = "";
        for (let i = 0; i < data.neurons.length; i++) {
            doRenderNewNode(data.neurons[i].name, null, 0, data.neurons[i].name);

            if (!data.neurons[i].synapses) data.neurons[i].synapses = [];
            if (!data.neurons[i].receptors) data.neurons[i].receptors = [];

            FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name] = data.neurons[i];
            for (let r = 0; r < FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name].receptors.length; r++) {
                FManager.getFrame(viewer).getData().neuron_list[data.neurons[i].name].receptors[r].data_scopes = [];
            }

            if (data.neurons[i].ensemble === undefined) {
                data.neurons[i].ensemble = "";
            }

            // console.log(data.neurons[i].ensemble)

            for (let j = 0; data.neurons[i].input_signals && j < data.neurons[i].input_signals.length; j++) {
                let found1 = data.entries.find((value) => data.neurons[i].input_signals[j] === value);
                let found2 = data.neurons.find((value) => data.neurons[i].input_signals[j] === value.name);

                if (found1 || found2) {
                    doRenderNewEdge(data.neurons[i].input_signals[j], data.neurons[i].name);
                }
            }
        }

        for (let i = 0; data.output_signals && i < data.output_signals.length; i++) {
            doRenderNewNode("Output "+(i+1), null, 2, "Output "+(i+1));
            doRenderNewEdge(data.output_signals[i], "Output "+(i+1));

            FManager.getFrame(viewer).getData().output_list.push({id: "Output "+(i+1), link: data.output_signals[i]});
        }

        FManager.getFrame(viewer).getData().network_info.name = data.name;
        FManager.getFrame(viewer).getData().network_info.desc = data.desc;
        FManager.getFrame(viewer).getData().network_info.version = data.version;
        FManager.getFrame(viewer).getData().network_info.parameter_count = data.parameter_count;
        FManager.getFrame(viewer).getData().network_info.model_size = data.model_size;

        setInterlinkConnectionName(data.name);

        console.log("done", performance.now());

        //if (full_update) FManager.getFrame(viewer).doInitViewer(elements, FManager.getFrame(viewer).getData().entry_list[0]);
        //else
        FManager.getFrame(viewer).doUpdateLayout();
        FManager.getFrame(viewer).getData().viewer_elements = FManager.getFrame(viewer).getViewer().save();
        FManager.getFrame(viewer).doRedrawLists();
        FManager.getFrame(viewer).doAddModelHistoryToList("Structure update");

        doCreateParameterList("", "background", viewer);
    } catch (e) {
        console.log(e);
    }
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
