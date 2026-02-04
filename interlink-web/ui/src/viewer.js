
let NodeWeightByType = {0: 24, 1: 15, 2: 15}
let NodeTypeNameByType = {0: "neuron", 1: "entry", 2: "output"}

let doHighlightNode = function(id) {
    FManager.getInterlinkFrame().getViewer().setNodeColor(id, getCSStyleColor("node-highlighted"));
    setTimeout(function() {
        FManager.getInterlinkFrame().getViewer().setNodeColor(id, getCSStyleColor("node"));
    }, 1000);
};

function doApplyStylesheet(viewer) {
    viewer.setAllNodesColor(getCSStyleColor("node"));
    viewer.setAllEdgesColor(getCSStyleColor("edge"));
    viewer.setAllLabelsColor(getCSStyleColor("label"));
    viewer.setSelectionColor(getCSStyleColor("node-selected"));
    viewer.setGridColor(getCSStyleColor("grid"));
    viewer.setBackgroundColor(getCSStyleColor("bg"));
}

function getCSStyleColor(item) {
    let theme = facefull.Themes.getCurrentThemeID();
    // console.log("getting viewer style", theme, item)
    switch (theme) {
        case 0:
            switch (item) {
                case "bg": return "#1E1F22";
                case "grid": return "#777777";
                case "node": return "#fff";
                case "node-highlighted": return "#ffa645";
                case "node-selected": return "#61bffc";
                case "edge": return "#fff";
                case "label": return "#f3f3f3";
            }
            break;
        case 1:
            switch (item) {
                case "bg": return "#f9f9f9";
                case "grid": return "#777777";
                case "node": return "#464646";
                case "node-highlighted": return "#ffa645";
                case "node-selected": return "#61bffc";
                case "edge": return "#464646";
                case "label": return "#5d5d5d";
            }
            break;
    }
}

function doInitViewerAttributes(viewer, startpoint = "", predefined = false, id = -1) {
    viewer.onSelectionChange((selectedIds) => {
        if (selectedIds.length === 0) {
            FManager.getCurrentFrame().doClearSelectedElements();
            doCreateParameterList("", "background");
        }
    });

    viewer.onNodeClick((id, data) => {
        try {
            FManager.getCurrentFrame().doClearSelectedElements();
            FManager.getCurrentFrame().doAddSelectedElement(data.name, data.type);
            doClearParameterList(FManager.getCurrentFrameID());
            console.log("node click", id, data)

            doCreateParameterList(data.name, NodeTypeNameByType[data.type]);
        } catch (e) {
            console.log(e);
        }
    });
    //
    // cy.on('select', function(evt) {
    //     try {
    //         let flag = (evt.target.data("weight"));
    //         if (flag === -1 || flag === 1 || flag === 2)
    //
    //     } catch (e) {
    //         console.log("on node select", e);
    //     }
    // });
    //
    // cy.on('unselect', function(evt) {
    //     try {
    //         let flag = (evt.target.data("weight"));
    //         if (flag === -1 || flag === 1 || flag === 2)
    //             FManager.getCurrentFrame().doRemoveSelectedElement(evt.target.id());
    //     } catch (e) {
    //         console.log("on node unselect", e);
    //     }
    // });
}

function doRenderNewNode(id, pos = null, type = 0, name = "") {
    if (name === "") name = id;
    FManager.getCurrentFrame().getViewer().addNode(id, 0, 0, { label: name, color: getCSStyleColor("node"), radius: NodeWeightByType[type], data: { name: name, type: type }});
}

function doRenderNewEdge(id1, id2) {
    FManager.getCurrentFrame().getViewer().addEdge(id1, id2);
    FManager.getCurrentFrame().getViewer().setEdgeColor(id1, id2, getCSStyleColor("edge"));
}

function doClearBackground() {

    // if (FManager.getCurrentFrame() && FManager.getCurrentFrame().getViewer().context && FManager.getCurrentFrame().getViewer().canvas) {
    //     FManager.getCurrentFrame().getViewer().context.fillStyle = getCSStyleColor("bg"); // page bg color
    //     FManager.getCurrentFrame().getViewer().context.fillRect(0, 0, FManager.getCurrentFrame().getViewer().canvas.width,
    //                                                                         FManager.getCurrentFrame().getViewer().canvas.height);
    // }
}
