
let FManager = null;

function FrameManager() {
    this.frames = [];
    this.current_frame = 0;
    this.interlink_frame = 0;

    this.doInit = function() {
        this.frames.push(new DefaultFrame(1));
    }

    this.doSwitchCurrentFrame = function(id) {
        this.current_frame = id;
    }

    this.getCurrentFrame = function() {
        return this.frames[this.current_frame];
    }

    this.getInterlinkFrame = function() {
        return this.frames[this.interlink_frame];
    }

    this.getFrame = function(id) {
        return this.frames[id];
    }

    this.getCurrentFrameID = function() {
        return this.current_frame;
    }

    this.getInterlinkFrameID = function() {
        return this.interlink_frame;
    }

    this.doInit();
}

function DefaultFrame(id) {
    this.id = id;
    this.viewer = null;
    this.string_data = "";
    this.data = {
        entry_list: [],
        neuron_list: {},
        output_list: [],
        network_info: {name: "", desc: "", version: ""}
    };
    this.neuron_model = null;
    this.leftpaneltop_state = -1;
    this.leftpaneltop_count = 3;
    this.leftpanelbottom_state = -1;
    this.rightpanel_state = -1;
    this.selected_elements = [];

    this.doInit = function() {
        console.log("frame init");
        document.getElementById("ELP"+this.id).classList.add("Hidden");
        document.getElementById("ILP"+this.id).classList.add("Hidden");
        document.getElementById("OLP"+this.id).classList.add("Hidden");
        document.getElementById("DRP"+this.id).classList.add("Hidden");
        this.data.entry_list = [];
        this.data.neuron_list = {};
        this.data.output_list = [];
        this.data.network_info = {name: "", desc: "", version: ""};
        this.doInitViewer();
    }

    this.doClearLists = function() {
        facefull.Lists["EL"+this.id].doClear();
        facefull.Lists["IL"+this.id].doClear();
        facefull.Lists["OL"+this.id].doClear();
        facefull.Scrollboxes["ELSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["ILSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["OLSB"+this.id].doUpdateScrollbar();
        this.data.entry_list = [];
        this.data.neuron_list = {};
        this.data.output_list = [];
        this.data.network_info = {name: "", desc: "", version: ""};
        this.string_data = "";
    }

    this.doUpdateString = function(str) {
        this.string_data = str;
    }

    this.doImportViewer = function(data) {
        // let name = "CS" + this.id;
        // let cs = cytoscape({
        //     container: document.getElementById(name),
        //     wheelSensitivity: 0.1,
        //     zoom: 0.6,
        // });
        // const bottomLayer = cs.cyCanvas({
        //     zIndex: -1
        // });
        // let canvas = bottomLayer.getCanvas();
        // let context = canvas.getContext("2d");
        // this.viewer = {
        //     cs: cs,
        //     canvas: canvas,
        //     context: context
        // }

        this.doUpdateViewer();
        this.viewer.cs.json(data);
        facefull.ItemPickers["DCP"].doSelect(facefull.ItemPickers["DCP"].getState());
        // doInitViewerAttributes(this.viewer.cs, "", true);
    }

    this.doInitViewer = function(elements = [], startpoint = "") {
        let name = "CS" + this.id;
        let cs = cytoscape({
            container: document.getElementById(name),

            boxSelectionEnabled: true,
            wheelSensitivity: 0.1,
            motionBlur: true,
            zoom: 0.9,
            // autounselectify: true,
            pan: { x: document.getElementById(name).offsetWidth/2, y: 0 },
            // selectionType: 'additive',

            elements: elements,
        });
        doApplyStylesheet(cs.style())
        cs.style().update();

        const bottomLayer = cs.cyCanvas({
            zIndex: -1
        });
        let canvas = bottomLayer.getCanvas();
        let context = canvas.getContext("2d");
        this.viewer = {
            cs: cs,
            canvas: canvas,
            context: context
        }

        doInitViewerAttributes(this.viewer.cs, startpoint, false, this.id);

        console.log("doInitViewer done", performance.now());
    }

    this.doUpdateViewer = function(elements = []) {
        console.log("update viewer");
        this.viewer.cs.remove("node");
        this.viewer.cs.add(elements);
        this.viewer.cs.layout(KlayLayoutOptions).run();
        let nodes = this.viewer.cs.filter('node');
        console.log("align", nodes)
        // nodes.select()
        //cy.emit('tap node', nodes);
    }

    this.doInitNeuronModel = function(e, data = [], size = 0) {
        if (this.neuron_model) this.neuron_model.destroy();
        e.innerHTML = "";
        this.neuron_model = new Chart(e, {
            type: 'bubble',
            data: data,
            options: getChartOptions(size),
        });
    }

    this.doUpdateNeuronModel = function(data, options) {
        this.neuron_model.config.data = data;
        this.neuron_model.config.options = options;
        this.neuron_model.update('none');
    }

    this.doClearNeuronModel = function(e) {
        if (this.neuron_model) this.neuron_model.destroy();
        e.innerHTML = "";
    }

    this.doAddSelectedElement = function(name) {
        if (name === "") return;

        let eid = this.selected_elements.findIndex(function(element) {
            return element === name;
        });

        if (eid === -1)
            this.selected_elements.push(name);

        console.log("added selected element", name, this.selected_elements)
    }

    this.doRemoveSelectedElement = function(name) {
        let eid = this.selected_elements.findIndex(function(element) {
            return element === name;
        });

        if (eid !== -1)
            this.selected_elements.splice(eid, 1);

        console.log("removed selected element", name, this.selected_elements)
    }

    this.doClearSelectedElements = function() {
        this.selected_elements = [];
        console.log("cleared selected elements")
    }

    this.setLeftPanelTopState = function(state) {
        this.leftpaneltop_state = state;
    }

    this.setLeftPanelBottomState = function(state) {
        this.leftpanelbottom_state = state;
    }

    this.setRightPanelState = function(state) {
        this.rightpanel_state = state;
    }

    this.getViewer = function() {
        return this.viewer;
    }

    this.getData = function() {
        return this.data;
    }

    this.getLeftPanelTopState = function() {
        return this.leftpaneltop_state;
    }

    this.getLeftPanelTopCount = function() {
        return this.leftpaneltop_count;
    }

    this.getLeftPanelBottomState = function() {
        return this.leftpanelbottom_state;
    }

    this.getRightPanelState = function() {
        return this.rightpanel_state;
    }

    this.getLastSelectedElement = function() {
        if (this.selected_elements.length > 0)
            return this.selected_elements[this.selected_elements.length-1];
        return "";
    }

    this.getString = function() {
        return this.string_data;
    }

    this.doInit();
}
