
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
    this.model_life_history = [];
    this.model_lifetime_list = [];
    this.neuron_model = null;
    this.leftpaneltop_state = -1;
    this.leftpaneltop_count = 3;
    this.leftpanelbottom_state = -1;
    this.rightpanel_state = -1;
    this.selected_elements = [];
    this.last_structure_id = -1;
    this.auto_update_scope = true;

    this.doInit = function() {
        console.log("frame init");
        document.getElementById("ELP"+this.id).classList.add("Hidden");
        document.getElementById("ILP"+this.id).classList.add("Hidden");
        document.getElementById("OLP"+this.id).classList.add("Hidden");
        document.getElementById("DRP"+this.id).classList.add("Hidden");
        this.doInitViewer();
    }

    this.doClearLists = function() {
        facefull.Lists["EL"+this.id].doClear();
        facefull.Lists["IL"+this.id].doClear();
        facefull.Lists["OL"+this.id].doClear();
        facefull.Scrollboxes["ELSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["ILSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["OLSB"+this.id].doUpdateScrollbar();
        this.string_data = "";
    }

    this.doCreateModel = function() {
        this.model_life_history.push({
            entry_list: [],
            neuron_list: {},
            output_list: [],
            timeline_data: [],
            viewer_elements: [],
            network_info: {name: "", desc: "", version: "", parameter_count: "", model_size: ""}
        });
    }

    this.doAddModelHistoryToList = function(str) {
        let id = this.model_life_history.length - 1;
        this.model_lifetime_list.push({structure_id: id, data_id: this.model_life_history[id].timeline_data.length-1, name: str});

        facefull.Lists["NMDL"].doAdd(["["+this.model_lifetime_list.length+"] "+str]);
        facefull.Scrollboxes["NMSB"].doUpdateScrollbar();

        if (this.auto_update_scope) {
            facefull.Lists["NMDL"].doSelect(facefull.Lists["NMDL"].getLength()-1);

            this.auto_update_scope = true; // redefine after list's doSelect
        }

        if (facefull.Lists["NMDL"].getState() === -1) this.last_structure_id = id;
    }

    this.doUpdateString = function(str) {
        this.string_data = str;
    }

    this.doImportViewer = function(data) {
        this.viewer.clear();
        this.viewer.load(data);
        doApplyStylesheet(this.viewer);
    }

    this.doInitViewer = function(elements = [], startpoint = "") {
        let name = "CS" + this.id;
        this.viewer = new GraphViewer(document.getElementById(name));

        doApplyStylesheet(this.viewer);

        doInitViewerAttributes(this.viewer, startpoint, false, this.id);

        console.log("doInitViewer done", performance.now());
    }

    this.doUpdateLayout = function() {
        this.viewer.autoLayout();
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

    this.doCheckObjectExists = function(name) {
        let current_data = this.getData(this.last_structure_id);
        return current_data.neuron_list[name] !== null && current_data.neuron_list[name] !== undefined
            || current_data.entry_list.find(function (e) {
                return e === name
            })
            || current_data.output_list.find(function (e) {
                return e === name
            });
    }

    this.doManageViewport = function(hid) {
        let h = this.getHistoryListItem(hid);
        let update = true;

        if (h.structure_id !== this.last_structure_id) {
            this.doImportViewer(this.getData(h.structure_id).viewer_elements);
            this.doRedrawLists(h.structure_id);

            this.selected_elements = [];
            this.last_structure_id = h.structure_id;
            update = false;
        }

        let e = this.getLastSelectedElement();
        if (e !== "") doCreateParameterList(e.name, NodeTypeNameByType[e.type], -1, update);
        else doCreateParameterList("", "background");
    }

    this.doClearModelHistory = function() {
        this.model_lifetime_list = [];
    }

    this.doRedrawLists = function(id = -1) {
        this.doClearLists();
        let ne_list = {};

        let current_data = this.getData(id);

        for (let e in current_data.entry_list) {
            facefull.Lists["IL"+this.id].doAdd([current_data.entry_list[e]]);
        }

        for (let n in current_data.neuron_list) {
            let ensemble = current_data.neuron_list[n].ensemble;
            if (!ne_list[ensemble]) ne_list[ensemble] = [];
            ne_list[ensemble].push(current_data.neuron_list[n].name);
        }

        for (let e in ne_list) {
            let en_name = e;
            if (en_name === "") en_name = "No ensemble";
            facefull.Lists["EL"+this.id].doAdd([en_name], 0, {action: "arrow"});
            for (let n in ne_list[e]) {
                facefull.Lists["EL"+this.id].doAdd([ne_list[e][n]], 1);
            }
        }

        for (let o in current_data.output_list) {
            facefull.Lists["OL"+this.id].doAdd(["Output "+(parseInt(o)+1)]);
        }

        facefull.Scrollboxes["ELSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["ILSB"+this.id].doUpdateScrollbar();
        facefull.Scrollboxes["OLSB"+this.id].doUpdateScrollbar();
    }

    this.doAddSelectedElement = function(name, type) {
        if (name === "") return;

        let eid = this.selected_elements.findIndex(function(element) {
            return element.name === name;
        });

        if (eid === -1)
            this.selected_elements.push({name: name, type: type});

        console.log("added selected element", name, this.selected_elements)
    }

    this.doRemoveSelectedElement = function(name) {
        let eid = this.selected_elements.findIndex(function(element) {
            return element.name === name;
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

    this.setAutoUpdateScope = function(update = true) {
        this.auto_update_scope = update;
    }

    this.getAutoUpdateScope = function() {
        return this.auto_update_scope;
    }

    this.getViewer = function() {
        return this.viewer;
    }

    this.getData = function(id = -1) {
        if (id === -1) return this.model_life_history[this.model_life_history.length-1];
        return this.model_life_history[id];
    }

    this.getHistoryListItem = function(id) {
        if (id === -1) return this.model_lifetime_list[this.model_lifetime_list.length-1];
        return this.model_lifetime_list[id];
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
