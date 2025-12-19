
let MouseMode = 0;
let MouseLinkFrom = "";

let doHighlightNode = function(name) {
    let node = FManager.getInterlinkFrame().getViewer().cs.filter('node[id = "'+name+'"]');
    node.addClass('highlighted');
    setTimeout(function() {
        node.removeClass('highlighted');
    }, 1000);
};

let KlayLayoutOptions = {
    name: 'klay',
    nodeDimensionsIncludeLabels: false, // Boolean which changes whether label dimensions are included when calculating node dimensions
    fit: false, // Whether to fit
    //padding: 20, // Padding on fit
    animate: false, // Whether to transition the node positions
    animateFilter: function( node, i ){ return true; }, // Whether to animate specific nodes when animation is on; non-animated nodes immediately go to their final positions
    animationDuration: 500, // Duration of animation in ms if enabled
    animationEasing: undefined, // Easing of animation if enabled
    transform: function( node, pos ){ return pos; }, // A function that applies a transform to the final node position
    ready: undefined, // Callback on layoutready
    stop: undefined, // Callback on layoutstop
    klay: {
        // Following descriptions taken from http://layout.rtsys.informatik.uni-kiel.de:9444/Providedlayout.html?algorithm=de.cau.cs.kieler.klay.layered
        addUnnecessaryBendpoints: false, // Adds bend points even if an edge does not change direction.
        aspectRatio: 1.6, // The aimed aspect ratio of the drawing, that is the quotient of width by height
        borderSpacing: 60, // Minimal amount of space to be left to the border
        compactComponents: true, // Tries to further compact components (disconnected sub-graphs).
        crossingMinimization: 'LAYER_SWEEP', // Strategy for crossing minimization.
        /* LAYER_SWEEP The layer sweep algorithm iterates multiple times over the layers, trying to find node orderings that minimize the number of crossings. The algorithm uses randomization to increase the odds of finding a good result. To improve its results, consider increasing the Thoroughness option, which influences the number of iterations done. The Randomization seed also influences results.
        INTERACTIVE Orders the nodes of each layer by comparing their positions before the layout algorithm was started. The idea is that the relative order of nodes as it was before layout was applied is not changed. This of course requires valid positions for all nodes to have been set on the input graph before calling the layout algorithm. The interactive layer sweep algorithm uses the Interactive Reference Point option to determine which reference point of nodes are used to compare positions. */
        cycleBreaking: 'GREEDY', // Strategy for cycle breaking. Cycle breaking looks for cycles in the graph and determines which edges to reverse to break the cycles. Reversed edges will end up pointing to the opposite direction of regular edges (that is, reversed edges will point left if edges usually point right).
        /* GREEDY This algorithm reverses edges greedily. The algorithm tries to avoid edges that have the Priority property set.
        INTERACTIVE The interactive algorithm tries to reverse edges that already pointed leftwards in the input graph. This requires node and port coordinates to have been set to sensible values.*/
        direction: 'RIGHT', // Overall direction of edges: horizontal (right / left) or vertical (down / up)
        /* UNDEFINED, RIGHT, LEFT, DOWN, UP */
        edgeRouting: 'POLYLINE', // Defines how edges are routed (POLYLINE, ORTHOGONAL, SPLINES)
        edgeSpacingFactor: 0.5, // Factor by which the object spacing is multiplied to arrive at the minimal spacing between edges.
        feedbackEdges: false, // Whether feedback edges should be highlighted by routing around the nodes.
        fixedAlignment: 'BALANCED', // Tells the BK node placer to use a certain alignment instead of taking the optimal result.  This option should usually be left alone.
        /* NONE Chooses the smallest layout from the four possible candidates.
        LEFTUP Chooses the left-up candidate from the four possible candidates.
        RIGHTUP Chooses the right-up candidate from the four possible candidates.
        LEFTDOWN Chooses the left-down candidate from the four possible candidates.
        RIGHTDOWN Chooses the right-down candidate from the four possible candidates.
        BALANCED Creates a balanced layout from the four possible candidates. */
        inLayerSpacingFactor: 1.0, // Factor by which the usual spacing is multiplied to determine the in-layer spacing between objects.
        layoutHierarchy: true, // Whether the selected layouter should consider the full hierarchy
        linearSegmentsDeflectionDampening: 0.3, // Dampens the movement of nodes to keep the diagram from getting too large.
        mergeEdges: false, // Edges that have no ports are merged so they touch the connected nodes at the same points.
        mergeHierarchyCrossingEdges: true, // If hierarchical layout is active, hierarchy-crossing edges use as few hierarchical ports as possible.
        nodeLayering:'INTERACTIVE', // Strategy for node layering.
        /* NETWORK_SIMPLEX This algorithm tries to minimize the length of edges. This is the most computationally intensive algorithm. The number of iterations after which it aborts if it hasn't found a result yet can be set with the Maximal Iterations option.
        LONGEST_PATH A very simple algorithm that distributes nodes along their longest path to a sink node.
        INTERACTIVE Distributes the nodes into layers by comparing their positions before the layout algorithm was started. The idea is that the relative horizontal order of nodes as it was before layout was applied is not changed. This of course requires valid positions for all nodes to have been set on the input graph before calling the layout algorithm. The interactive node layering algorithm uses the Interactive Reference Point option to determine which reference point of nodes are used to compare positions. */
        nodePlacement:'BRANDES_KOEPF', // Strategy for Node Placement
        /* BRANDES_KOEPF Minimizes the number of edge bends at the expense of diagram size: diagrams drawn with this algorithm are usually higher than diagrams drawn with other algorithms.
        LINEAR_SEGMENTS Computes a balanced placement.
        INTERACTIVE Tries to keep the preset y coordinates of nodes from the original layout. For dummy nodes, a guess is made to infer their coordinates. Requires the other interactive phase implementations to have run as well.
        SIMPLE Minimizes the area at the expense of... well, pretty much everything else. */
        randomizationSeed: 0, // Seed used for pseudo-random number generators to control the layout algorithm; 0 means a new seed is generated
        routeSelfLoopInside: false, // Whether a self-loop is routed around or inside its node.
        separateConnectedComponents: true, // Whether each connected component should be processed separately
        spacing: 60, // Overall setting for the minimal amount of space to be left between objects
        thoroughness: 7 // How much effort should be spent to produce a nice layout..
    },
    priority: function( edge ){ return null; }, // Edges with a non-nil value are skipped when greedy edge cycle breaking is enabled
};

let PresetLayoutOptions = {
    name: 'preset',
};

function doApplyStylesheet(object) {
    object.selector('node')
        .style({
            'content': 'data(name)',
            'background-color': getCSStyleColor("node"),
            'color': getCSStyleColor("label")
        })
        .selector('.viewer-node-small')
        .style({
            'width': 20,
            'height': 20
        })
        .selector('edge')
        .style({
            'curve-style': 'bezier',
            'control-point-step-size': 40,
            // 'control-point-distance': 1,
            'target-arrow-shape': 'vee',
            'width': 2,
            'line-color': getCSStyleColor("edge"),
            'target-arrow-color': getCSStyleColor("edge")
        })
        .selector('.highlighted')
        .style({
            'background-color': '#ff986c',
            'line-color': '#ffa645',
            'target-arrow-color': '#b275b6',
            'transition-property': 'background-color, line-color, target-arrow-color',
            'transition-duration': '0.5s'
        })
        .selector(':selected')
        .style({
            'background-color': '#61bffc',
        })
        .selector(':parent')
        .style({
            'text-valign': 'top',
            'text-halign': 'center',
            'background-color': 'rgb(98,98,98)',
            'background-opacity': '0.1',
            'shape' : 'roundrectangle',
            'border-width' : '0',
        })
}

function getCSStyleColor(item) {
    let theme = facefull.Themes.getCurrentThemeID();
    // console.log("getting viewer style", theme, item)
    switch (theme) {
        case 0:
            switch (item) {
                case "bg": return "#1E1F22";
                case "grid": return "#2c2c2c";
                case "node": return "#fff";
                case "edge": return "#fff";
                case "label": return "#f3f3f3";
            }
            break;
        case 1:
            switch (item) {
                case "bg": return "#f9f9f9";
                case "grid": return "#eaeaea";
                case "node": return "#464646";
                case "edge": return "#464646";
                case "label": return "#5d5d5d";
            }
            break;
    }
}

let InterlinkContextMenuOptions = {
    // Customize event to bring up the context menu
    // Possible options https://js.cytoscape.org/#events/user-input-device-events
    evtType: 'cxttap',
    // List of initial menu items
    // A menu item must have either onClickFunction or submenu or both
    menuItems: [
        {
            id: 'copy',
            content: 'Copy',
            selector: 'node',
            onClickFunction: function () {
                doCheckNeuronForCopy(FManager.getCurrentFrame().selected_elements, FManager.getInterlinkFrameID());
            },
            disabled: false
        }
    ],
    // css classes that menu items will have
    menuItemClasses: [
        // add class names to this list
    ],
    // css classes that context menu will have
    contextMenuClasses: [
        // add class names to this list
    ],
    // Indicates that the menu item has a submenu. If not provided default one will be used
    // submenuIndicator: { src: 'assets/submenu-indicator-default.svg', width: 12, height: 12 }
};

function doApplyGrid(object) {
    object.gridGuide({
        panGrid: true,
        gridSpacing: 60,
        gridColor: getCSStyleColor("grid"),
        snapToGridOnRelease: false,
        snapToGridCenter: false,
    });

    // object.snapToGrid({
    //     gridSpacing: 50,
    //     strokeStyle: getCSStyleColor("grid"),
    // });
    // object.snapToGrid('snapOn');
}

function doInitViewerAttributes(cy, startpoint = "", predefined = false, id = -1) {
    if (!predefined) {
        cy.layout(KlayLayoutOptions).run();
    } else {
        cy.layout(PresetLayoutOptions).run();
    }

    cy.ready(evt => {
        if (startpoint !== "") {
            let node = cy.filter('node[id = "'+startpoint+'"]');
            let position = node.position();
            console.log(position);
            cy.zoom(1);
            cy.pan({ x: position.x, y: -position.y+(cy.height()/2) });
        }

        setTimeout(function() {
            // if (id === 1) {
            //     try {
            //         cy.contextMenus(InterlinkContextMenuOptions);
            //     } catch (e) {
            //         console.log("Error initializing viewer context menus", e);
            //     }
            // }

            doApplyGrid(cy);

            // let nodes = cy.filter('node');
            // console.log("align", nodes)
            // nodes.align("top", "left")
        }, 1);
    });

    // cy.on("render cyCanvas.resize", evt => {
    //     bottomLayer.resetTransform(ctx);
    //     bottomLayer.clear(ctx);
    //     bottomLayer.setTransform(ctx);
    //
    //     ctx.save();
    //     let wx = window.innerWidth;
    //     let hx = window.innerHeight;
    //     for (let x = -wx; x < wx+100; x += 50) {
    //         for (let y = -hx; y < hx+100; y += 50) {
    //             let roundedX = Math.round(x);
    //             let roundedY = Math.round(y);
    //             ctx.fillStyle = '#a55';
    //             ctx.fillRect(roundedX, roundedY, 2, 2);
    //         }
    //     }
    //     }
    //     ctx.restore();
    // });

    cy.on('mousemove', function(evt) {
        if (MouseLinkFrom !== "")
            doDrawEdgeLine(MouseLinkFrom, {x: evt.renderedPosition.x, y: evt.renderedPosition.y});
        // console.log( 'move ',  evt.renderedPosition.x,  evt.renderedPosition.y );
    });

    cy.on('drag', function(evt) {
        if (FManager.getCurrentFrameID() === 0)
            setProjectChanged();
    });

    // cy.on('tap', 'node', function(evt) {
    //     let node = evt.target;
    //     console.log('tapped', node.id(), node.data('weight'));
    // });

    cy.on('cxttap', function(evt) {
        let node = evt.target;

        // FManager.getCurrentFrame().doClearSelectedElements();
        if (!node.id) {
            doCreateParameterList("", "background");
            let list = [...FManager.getCurrentFrame().selected_elements];
            for (let i in list) {
                FManager.getCurrentFrame().getViewer().cs.filter('[id = "'+list[i]+'"]').unselect();
            }
        } else {
            try {
                let citem = FManager.getCurrentFrame().selected_elements.findIndex(function(element) {
                    return element === node.id();
                });
                if (citem === -1) {
                    let list = [...FManager.getCurrentFrame().selected_elements];
                    for (let i in list) {
                        FManager.getCurrentFrame().getViewer().cs.filter('[id = "'+list[i]+'"]').unselect();
                    }
                }
                node.select();
                doClearParameterList(FManager.getCurrentFrameID());
                switch (node.data('weight')) {
                    case -1:
                        doCreateParameterList(node.id());
                        break;
                    case 1:
                        doCreateParameterList(node.id(), "entry");
                        break;
                    case 2:
                        doCreateParameterList(node.id(), "output");
                        break;
                }
            } catch (e) {
                console.log(e);
            }
        }
    });

    cy.on('tap', function(evt) {
        let node = evt.target;
        console.log("tapped on viewer", node.id)

        if (!node.id) { // check if tapped element is background or node
            PastePos = {x: evt.renderedPosition.x, y: evt.renderedPosition.y};
            switch (MouseMode) {
                case 1:
                    doAddNode({x: evt.renderedPosition.x, y: evt.renderedPosition.y}, "", 1);
                    break;
                case 2:
                    doAddNode({x: evt.renderedPosition.x, y: evt.renderedPosition.y}, "", 0);
                    break;
                case 3:
                    doAddNode({x: evt.renderedPosition.x, y: evt.renderedPosition.y}, "", 2);
                    break;
            }
            doCreateParameterList("", "background");
            // FManager.getCurrentFrame().doClearSelectedElements();
        } else {
            if (MouseMode === 4) {
                if (MouseLinkFrom === "") {
                    MouseLinkFrom = node.id();
                    return;
                } else {
                    let eobj = FManager.getCurrentFrame().getData().entry_list.find(function (el) {
                        return el === node.id();
                    });
                    // let oobj = OutputList.find(function(el) {
                    //     return el === node.id();
                    // });
                    let nobj = FManager.getCurrentFrame().getData().neuron_list[node.id()];
                    let onode = cy.filter('node[id = "'+MouseLinkFrom+'"]');
                    let edges = cy.edges('[id = "'+MouseLinkFrom+"-"+node.id()+'"]');
                    console.log(eobj, nobj);

                    if (eobj || onode.data('weight') === 2 || (onode.data('weight') === 1 && node.data('weight') === 2)) {
                        AlertShow("Error", "Can't create this link. Accepted only 'neuron->neuron', 'neuron->output' and 'entry->neuron' links.", "error");
                    } else if (node.data('weight') === 2) {
                        // let state = false;
                        // for (let n in FManager.getCurrentFrame().getData().neuron_list) {
                        //     let eni = FManager.getCurrentFrame().getData().neuron_list[n].input_signals.find(function (el) {
                        //         return el === node.id();
                        //     });
                        //     if (eni) {
                        //         state = true;
                        //         break;
                        //     }
                        // }
                        let oedges = cy.edges('[target = "'+node.id()+'"]');
                        if (oedges.length) {
                            AlertShow("Error", "This output already used.", "error");
                        } else if (!edges.length) {
                            doAddLink(MouseLinkFrom, node.id());
                            let oitem = FManager.getCurrentFrame().getData().output_list.findIndex(function(element) {
                                return element.id === node.id();
                            })
                            FManager.getCurrentFrame().getData().output_list[oitem].link = MouseLinkFrom;
                        } else
                            AlertShow("Error", "Link already created.", "error");
                    } else if (nobj !== undefined && nobj !== null) {
                        console.log("create link (entry->neuron) from", MouseLinkFrom, onode.data('weight'))
                        if (edges.length)
                            AlertShow("Error", "Link already created.", "error");
                        else if (onode.length && onode.data('weight') !== 2) {
                            FManager.getCurrentFrame().getData().neuron_list[node.id()].input_signals.push(MouseLinkFrom);
                            doAddLink(MouseLinkFrom, node.id());
                        } else
                            AlertShow("Error", "Can't create this link. Accepted only 'neuron->neuron', 'neuron->output' and 'entry->neuron' links.", "error");
                    }

                    doClearBackground();
                    MouseLinkFrom = "";
                    doRefreshProjectStats();
                    setProjectChanged();
                }
            }

            try {
                doClearParameterList(FManager.getCurrentFrameID());
                switch (node.data('weight')) {
                    case -1:
                        doCreateParameterList(node.id());
                        break;
                    case 1:
                        doCreateParameterList(node.id(), "entry");
                        break;
                    case 2:
                        doCreateParameterList(node.id(), "output");
                        break;
                }

                // if (FManager.getCurrentFrameID() === FManager.getInterlinkFrameID())
                //     doInitMetricsShowRanges(node.id());
            } catch (e) {
                console.log(e);
            }

            console.log(node.position());
        }

        // console.log('tap',  evt.renderedPosition.x,  evt.renderedPosition.y, MouseMode);
    });

    cy.on('select', function(evt) {
        try {
            let flag = (evt.target.data("weight"));
            if (flag === -1 || flag === 1 || flag === 2)
                FManager.getCurrentFrame().doAddSelectedElement(evt.target.id());
        } catch (e) {
            console.log("on node select", e);
        }
    });

    cy.on('unselect', function(evt) {
        try {
            let flag = (evt.target.data("weight"));
            if (flag === -1 || flag === 1 || flag === 2)
                FManager.getCurrentFrame().doRemoveSelectedElement(evt.target.id());
        } catch (e) {
            console.log("on node unselect", e);
        }
    });
}

function doAddLink(from, to) {
    console.log("add link", from, to);
    let data = {
        group: 'edges',
        data: {
            id: from+"-"+to,
            source: from,
            target: to
        }
    }
    FManager.getCurrentFrame().getViewer().cs.add(data);
    setProjectChanged();
}

function doDeleteLink(from, to) {
    console.log("delete link", from, to);
    let edges = FManager.getCurrentFrame().getViewer().cs.edges('[id = "'+from+"-"+to+'"]');
    console.log(edges);
    for (let i = 0; i < edges.length; i++) {
        edges[i].remove();
    }
    setProjectChanged();
}

function doDrawEdgeLine(from, topos) {
    let nfrompos = FManager.getCurrentFrame().getViewer().cs.filter('node[id = "'+from+'"]').renderedPosition();
    doClearBackground();

    // bottomLayer.resetTransform(ctx);
    // bottomLayer.clear(ctx);
    // bottomLayer.setTransform(ctx);
    // ctx.save();

//     ctx.save();
//
// // Use the identity matrix while clearing the canvas
// //     ctx.setTransform(1, 0, 0, 1, 0, 0);
//     ctx.clearRect(0, 0, canvas.width, canvas.height);

// Restore the transform
//     ctx.restore();

    // ctx.save();
    // ctx.globalCompositeOperation = 'copy';
    // ctx.strokeStyle = 'transparent';
    // ctx.beginPath();
    // ctx.lineTo(0, 0);
    // ctx.stroke();
    // ctx.restore();


    FManager.getCurrentFrame().getViewer().context.strokeStyle = 'white';
    FManager.getCurrentFrame().getViewer().context.lineWidth = 3;

    // draw a red line
    FManager.getCurrentFrame().getViewer().context.beginPath();
    FManager.getCurrentFrame().getViewer().context.moveTo(nfrompos.x, nfrompos.y);
    FManager.getCurrentFrame().getViewer().context.lineTo(topos.x, topos.y);
    FManager.getCurrentFrame().getViewer().context.stroke();
    //ctx.restore();
}

function doRenderNewNode(id, pos = null, type = 0, name = "") {
    let data;
    if (name === "") name = id;
    if (pos) {
        data = {
            group: 'nodes',
            data: { id: id, name: name },
            renderedPosition: {x: pos.x, y: pos.y},
            selectable: true,
            grabbable: true,
        }
    } else {
        data = {
            group: 'nodes',
            data: { id: id, name: name },
            selectable: true,
            grabbable: true,
        }
    }
    if (type === 1 || type === 2) {
        data.classes = 'viewer-node-small';
        data.data.weight = type;
    } else {
        data.classes = 'viewer-node-full';
        data.data.weight = -1;
    }
    console.log("render", data);
    FManager.getCurrentFrame().getViewer().cs.add(data);
}

function doChangeMouseMode(mode) {
    console.log()
    MouseMode = mode;
    doClearBackground();
    MouseLinkFrom = "";
}

function doClearBackground() {
    if (FManager.getCurrentFrame() && FManager.getCurrentFrame().getViewer().context && FManager.getCurrentFrame().getViewer().canvas) {
        FManager.getCurrentFrame().getViewer().context.fillStyle = getCSStyleColor("bg"); // page bg color
        FManager.getCurrentFrame().getViewer().context.fillRect(0, 0, FManager.getCurrentFrame().getViewer().canvas.width,
                                                                            FManager.getCurrentFrame().getViewer().canvas.height);
    }
}
