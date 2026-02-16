/////////////////////////////////////////////////////////////////////////////
// Name:        graph-viewer.js
// Purpose:     Graph Viewer module
// Author:      Nickolay Babich
// Created:     02.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

function GraphViewer(container, options) {
    options = options || {};
    
    this.container = container;
    this.options = {
        gridSpacing: options.gridSpacing || this._getCSSVar('--grid-spacing', 50)
    };
    for (var key in options) {
        if (options.hasOwnProperty(key)) {
            this.options[key] = options[key];
        }
    }

    this.nodes = new Map();
    this.edges = [];
    
    this.transform = {
        offsetX: 0,
        offsetY: 0,
        scale: 1
    };
    
    this.minScale = 0.1;
    this.maxScale = 5;
    
    this.isDragging = false;
    this.wasDragged = false;
    this.lastMouseX = 0;
    this.lastMouseY = 0;
    
    this.nodeClickCallbacks = [];
    this.selectionChangeCallbacks = [];
    
    this.selectedNodes = new Set();
    
    this.gridColorOverride = null;
    this.autoRender = true;
    
    this._createCanvas();
    this._bindEvents();
    this._readCSSStyles();
    
    var self = this;
    setTimeout(function() {
        self._resizeCanvas();
        self.transform.offsetX = self.displayWidth / 2;
        self.transform.offsetY = self.displayHeight / 2;
        self.render();
    }, 0);
}

GraphViewer.prototype._getCSSVar = function(name, defaultValue) {
    var value = getComputedStyle(document.documentElement).getPropertyValue(name).trim();
    if (value === '') return defaultValue;
    var numValue = parseFloat(value);
    return isNaN(numValue) ? value : numValue;
};

GraphViewer.prototype._readCSSStyles = function() {
    this.styles = {
        gridColor: this._getCSSVar('--grid-color', '#cccccc'),
        gridDotSize: this._getCSSVar('--grid-dot-size', 2),
        gridSpacing: this._getCSSVar('--grid-spacing', 80),
        nodeFill: this._getCSSVar('--node-fill', '#4a90d9'),
        nodeStroke: this._getCSSVar('--node-stroke', '#2c5282'),
        nodeStrokeWidth: this._getCSSVar('--node-stroke-width', 0),
        nodeTextColor: this._getCSSVar('--node-text-color', '#ffffff'),
        nodeRadius: this._getCSSVar('--node-radius', 24),
        nodeFontSize: this._getCSSVar('--node-font-size', 16),
        nodeLabelOffset: this._getCSSVar('--node-label-offset', 12),
        nodeLabelColor: this._getCSSVar('--node-label-color', '#2c5282'),
        nodeSelectionColor: this._getCSSVar('--node-selection-color', '#ff6b6b'),
        edgeColor: this._getCSSVar('--edge-color', '#666666'),
        edgeWidth: this._getCSSVar('--edge-width', 2),
        edgeArrowSize: this._getCSSVar('--edge-arrow-size', 14),
        backgroundColor: this._getCSSVar('--background-color', '#ffffff')
    };
};

GraphViewer.prototype._createCanvas = function() {
    var self = this;
    
    this.canvas = document.createElement('canvas');
    this.canvas.style.display = 'block';
    this.ctx = this.canvas.getContext('2d');
    this.container.appendChild(this.canvas);
    
    this._resizeCanvas();
    
    this._resizeHandler = function() {
        self._resizeCanvas();
        self.render();
    };
    window.addEventListener('resize', this._resizeHandler);
};

GraphViewer.prototype._resizeCanvas = function() {
    this.canvas.style.width = '0';
    this.canvas.style.height = '0';
    
    var width = this.container.clientWidth;
    var height = this.container.clientHeight;
    var dpr = window.devicePixelRatio || 1;
    
    this.canvas.width = width * dpr;
    this.canvas.height = height * dpr;
    this.canvas.style.width = width + 'px';
    this.canvas.style.height = height + 'px';
    
    this.ctx.scale(dpr, dpr);
    this.displayWidth = width;
    this.displayHeight = height;
};

GraphViewer.prototype._bindEvents = function() {
    var self = this;
    
    this.canvas.addEventListener('wheel', function(e) {
        e.preventDefault();
        self._handleZoom(e);
    }, { passive: false });
    
    this.canvas.addEventListener('mousedown', function(e) {
        if (e.button === 0) {
            self.isDragging = true;
            self.wasDragged = false;
            self.lastMouseX = e.clientX;
            self.lastMouseY = e.clientY;
            self.canvas.style.cursor = 'grabbing';
        }
    });
    
    this.canvas.addEventListener('mousemove', function(e) {
        if (self.isDragging) {
            var dx = e.clientX - self.lastMouseX;
            var dy = e.clientY - self.lastMouseY;
            
            if (dx !== 0 || dy !== 0) {
                self.wasDragged = true;
            }
            
            self.transform.offsetX += dx;
            self.transform.offsetY += dy;
            
            self.lastMouseX = e.clientX;
            self.lastMouseY = e.clientY;
            
            self.render();
        }
    });
    
    this.canvas.addEventListener('mouseup', function() {
        self.isDragging = false;
        self.canvas.style.cursor = 'default';
    });
    
    this.canvas.addEventListener('mouseleave', function() {
        self.isDragging = false;
        self.canvas.style.cursor = 'default';
    });
    
    this.canvas.addEventListener('click', function(e) {
        if (self.wasDragged) {
            self.wasDragged = false;
            return;
        }
        
        var clickedNode = self._getNodeAtPosition(e.clientX, e.clientY);
        
        if (clickedNode) {
            self.selectNode(clickedNode.id);
            
            self.nodeClickCallbacks.forEach(function(callback) {
                callback(clickedNode.id, clickedNode.data);
            });
        } else {
            self.deselectAll();
        }
    });
};

GraphViewer.prototype._handleZoom = function(e) {
    var rect = this.canvas.getBoundingClientRect();
    var mouseX = e.clientX - rect.left;
    var mouseY = e.clientY - rect.top;
    
    var worldX = (mouseX - this.transform.offsetX) / this.transform.scale;
    var worldY = (mouseY - this.transform.offsetY) / this.transform.scale;
    
    var zoomFactor = e.deltaY > 0 ? 0.9 : 1.1;
    var newScale = Math.max(this.minScale, Math.min(this.maxScale, this.transform.scale * zoomFactor));
    
    this.transform.offsetX = mouseX - worldX * newScale;
    this.transform.offsetY = mouseY - worldY * newScale;
    this.transform.scale = newScale;
    
    this.render();
};

GraphViewer.prototype._screenToWorld = function(screenX, screenY) {
    var rect = this.canvas.getBoundingClientRect();
    var x = (screenX - rect.left - this.transform.offsetX) / this.transform.scale;
    var y = (screenY - rect.top - this.transform.offsetY) / this.transform.scale;
    return { x: x, y: y };
};

GraphViewer.prototype._worldToScreen = function(worldX, worldY) {
    var x = worldX * this.transform.scale + this.transform.offsetX;
    var y = worldY * this.transform.scale + this.transform.offsetY;
    return { x: x, y: y };
};

GraphViewer.prototype.snapToGrid = function(x, y) {
    var spacing = this.styles.gridSpacing;
    return {
        x: Math.round(x / spacing) * spacing,
        y: Math.round(y / spacing) * spacing
    };
};

GraphViewer.prototype._getNodeAtPosition = function(screenX, screenY) {
    var world = this._screenToWorld(screenX, screenY);
    var self = this;
    var result = null;
    
    this.nodes.forEach(function(node, id) {
        if (result) return;
        
        var radius = node.radius != null ? node.radius : self.styles.nodeRadius;
        var dx = world.x - node.x;
        var dy = world.y - node.y;
        var distance = Math.sqrt(dx * dx + dy * dy);
        
        if (distance <= radius) {
            result = {
                id: id,
                x: node.x,
                y: node.y,
                label: node.label,
                color: node.color,
                strokeColor: node.strokeColor,
                radius: node.radius,
                data: node.data
            };
        }
    });
    
    return result;
};

GraphViewer.prototype._getNodeRadius = function(node) {
    return node.radius != null ? node.radius : this.styles.nodeRadius;
};

GraphViewer.prototype.addNode = function(id, gridX, gridY, options) {
    options = options || {};
    var spacing = this.styles.gridSpacing;
    var node = {
        x: gridX * spacing,
        y: gridY * spacing,
        label: options.label || id,
        color: options.color || null,
        strokeColor: options.strokeColor || null,
        radius: options.radius != null ? options.radius : null,
        data: options.data || {}
    };
    
    this.nodes.set(id, node);
    this._autoRender();
    return this;
};

GraphViewer.prototype.removeNode = function(id) {
    this.nodes.delete(id);
    this.edges = this.edges.filter(function(edge) {
        return edge.from !== id && edge.to !== id;
    });
    this._autoRender();
    return this;
};

GraphViewer.prototype.setNodeColor = function(id, color) {
    var node = this.nodes.get(id);
    if (node) {
        node.color = color;
        this._autoRender();
    }
    return this;
};

GraphViewer.prototype.setAllNodesColor = function(color) {
    this.nodes.forEach(function(node) {
        node.color = color;
    });
    this._autoRender();
    return this;
};

GraphViewer.prototype.setNodeRadius = function(id, radius) {
    var node = this.nodes.get(id);
    if (node) {
        node.radius = radius;
        this._autoRender();
    }
    return this;
};

GraphViewer.prototype.getNode = function(id) {
    return this.nodes.get(id);
};

GraphViewer.prototype.getNodeData = function(id) {
    var node = this.nodes.get(id);
    return node ? node.data : null;
};

GraphViewer.prototype.getOccupiedGridPositions = function() {
    var positions = new Set();
    var spacing = this.styles.gridSpacing;
    
    this.nodes.forEach(function(node, id) {
        var gridX = Math.round(node.x / spacing);
        var gridY = Math.round(node.y / spacing);
        positions.add(gridX + ',' + gridY);
    });
    
    return positions;
};

GraphViewer.prototype.isPositionOccupied = function(gridX, gridY) {
    var positions = this.getOccupiedGridPositions();
    return positions.has(gridX + ',' + gridY);
};

GraphViewer.prototype.getGraphBounds = function() {
    if (this.nodes.size === 0) {
        return { minX: 0, maxX: 0, minY: 0, maxY: 0 };
    }
    
    var spacing = this.styles.gridSpacing;
    var minX = Infinity, maxX = -Infinity;
    var minY = Infinity, maxY = -Infinity;
    
    this.nodes.forEach(function(node, id) {
        var gridX = Math.round(node.x / spacing);
        var gridY = Math.round(node.y / spacing);
        minX = Math.min(minX, gridX);
        maxX = Math.max(maxX, gridX);
        minY = Math.min(minY, gridY);
        maxY = Math.max(maxY, gridY);
    });
    
    return { minX: minX, maxX: maxX, minY: minY, maxY: maxY };
};

GraphViewer.prototype.findFreePosition = function() {
    var bounds = this.getGraphBounds();
    var occupied = this.getOccupiedGridPositions();
    
    var startY = this.nodes.size === 0 ? 0 : bounds.maxY + 2;
    var startX = this.nodes.size === 0 ? 0 : bounds.minX;
    
    for (var y = startY; y < startY + 20; y++) {
        for (var x = startX; x < startX + 20; x++) {
            if (!occupied.has(x + ',' + y)) {
                return { gridX: x, gridY: y };
            }
        }
    }
    
    return { gridX: startX, gridY: startY };
};

GraphViewer.prototype.addNodeAuto = function(id, options) {
    var pos = this.findFreePosition();
    this.addNode(id, pos.gridX, pos.gridY, options);
    return pos;
};

GraphViewer.prototype.getConnectedComponents = function() {
    var self = this;
    var visited = new Set();
    var components = [];
    
    var adjacency = new Map();
    this.nodes.forEach(function(node, id) {
        adjacency.set(id, new Set());
    });
    
    for (var i = 0; i < this.edges.length; i++) {
        var edge = this.edges[i];
        if (adjacency.has(edge.from) && adjacency.has(edge.to)) {
            adjacency.get(edge.from).add(edge.to);
            adjacency.get(edge.to).add(edge.from);
        }
    }
    
    this.nodes.forEach(function(node, startId) {
        if (visited.has(startId)) return;
        
        var component = [];
        var queue = [startId];
        
        while (queue.length > 0) {
            var nodeId = queue.shift();
            if (visited.has(nodeId)) continue;
            
            visited.add(nodeId);
            component.push(nodeId);
            
            adjacency.get(nodeId).forEach(function(neighbor) {
                if (!visited.has(neighbor)) {
                    queue.push(neighbor);
                }
            });
        }
        
        components.push(component);
    });
    
    return components;
};

GraphViewer.prototype._topologicalSort = function(nodeIds) {
    var self = this;
    var nodeSet = new Set(nodeIds);
    var inDegree = new Map();
    var adjacency = new Map();
    
    for (var i = 0; i < nodeIds.length; i++) {
        var id = nodeIds[i];
        inDegree.set(id, 0);
        adjacency.set(id, []);
    }
    
    for (var j = 0; j < this.edges.length; j++) {
        var edge = this.edges[j];
        if (nodeSet.has(edge.from) && nodeSet.has(edge.to)) {
            adjacency.get(edge.from).push(edge.to);
            inDegree.set(edge.to, inDegree.get(edge.to) + 1);
        }
    }
    
    var levels = [];
    var assigned = new Set();
    
    while (assigned.size < nodeIds.length) {
        var level = [];
        
        for (var k = 0; k < nodeIds.length; k++) {
            var nodeId = nodeIds[k];
            if (assigned.has(nodeId)) continue;
            if (inDegree.get(nodeId) === 0) {
                level.push(nodeId);
            }
        }
        
        if (level.length === 0) {
            for (var m = 0; m < nodeIds.length; m++) {
                if (!assigned.has(nodeIds[m])) {
                    level.push(nodeIds[m]);
                    break;
                }
            }
        }
        
        for (var n = 0; n < level.length; n++) {
            var levelId = level[n];
            assigned.add(levelId);
            var neighbors = adjacency.get(levelId);
            for (var p = 0; p < neighbors.length; p++) {
                var neighbor = neighbors[p];
                inDegree.set(neighbor, inDegree.get(neighbor) - 1);
            }
        }
        
        levels.push(level);
    }
    
    return levels;
};

GraphViewer.prototype.layoutComponent = function(nodeIds, startX, startY, gapX, gapY) {
    if (startX === undefined) startX = 0;
    if (startY === undefined) startY = 0;
    if (gapX === undefined) gapX = 2;
    if (gapY === undefined) gapY = 2;
    
    var levels = this._topologicalSort(nodeIds);
    var spacing = this.styles.gridSpacing;
    
    var x = startX;
    for (var i = 0; i < levels.length; i++) {
        var level = levels[i];
        var y = startY - Math.floor((level.length - 1) * gapY / 2);
        
        for (var j = 0; j < level.length; j++) {
            var nodeId = level[j];
            var node = this.nodes.get(nodeId);
            if (node) {
                node.x = x * spacing;
                node.y = y * spacing;
            }
            y += gapY;
        }
        x += gapX;
    }
    
    this._autoRender();
    return this;
};

GraphViewer.prototype.autoLayout = function(gapX, gapY) {
    if (gapX === undefined) gapX = 2;
    if (gapY === undefined) gapY = 2;
    
    var components = this.getConnectedComponents();
    var currentY = 0;
    
    for (var i = 0; i < components.length; i++) {
        var component = components[i];
        var levels = this._topologicalSort(component);
        var maxNodesInLevel = 0;
        for (var j = 0; j < levels.length; j++) {
            var level = levels[j];
            maxNodesInLevel = Math.max(maxNodesInLevel, level.length);
        }
        
        var componentHeight = (maxNodesInLevel - 1) * gapY;
        
        var startY = currentY + Math.floor(componentHeight / 2);
        this.layoutComponent(component, 0, startY, gapX, gapY);
        
        currentY += componentHeight + gapY + 1;
    }
    
    this._autoRender();
    return this;
};

GraphViewer.prototype.addEdge = function(fromId, toId, options) {
    options = options || {};
    var edge = {
        from: fromId,
        to: toId,
        color: options.color || null,
        label: options.label || null
    };
    
    this.edges.push(edge);
    this._autoRender();
    return this;
};

GraphViewer.prototype.removeEdge = function(fromId, toId) {
    this.edges = this.edges.filter(function(edge) {
        return !(edge.from === fromId && edge.to === toId);
    });
    this._autoRender();
    return this;
};

GraphViewer.prototype.setEdgeColor = function(fromId, toId, color) {
    var edge = this.edges.find(function(e) {
        return e.from === fromId && e.to === toId;
    });
    if (edge) {
        edge.color = color;
        this._autoRender();
    }
    return this;
};

GraphViewer.prototype.setAllEdgesColor = function(color) {
    for (var i = 0; i < this.edges.length; i++) {
        this.edges[i].color = color;
    }
    this._autoRender();
    return this;
};

GraphViewer.prototype.setGridColor = function(color) {
    this.gridColorOverride = color;
    this._autoRender();
    return this;
};

GraphViewer.prototype.setBackgroundColor = function(color) {
    this.styles.backgroundColor = color;
    this._autoRender();
    return this;
};

GraphViewer.prototype.setAllLabelsColor = function(color) {
    this.styles.nodeLabelColor = color;
    this._autoRender();
    return this;
};

GraphViewer.prototype.onNodeClick = function(callback) {
    this.nodeClickCallbacks.push(callback);
    return this;
};

GraphViewer.prototype.onSelectionChange = function(callback) {
    this.selectionChangeCallbacks.push(callback);
    return this;
};

GraphViewer.prototype.selectNode = function(id) {
    if (!this.nodes.has(id)) return this;
    
    this.selectedNodes.clear();
    this.selectedNodes.add(id);
    
    this._notifySelectionChange();
    this._autoRender();
    return this;
};

GraphViewer.prototype.addToSelection = function(id) {
    if (!this.nodes.has(id)) return this;
    
    this.selectedNodes.add(id);
    
    this._notifySelectionChange();
    this._autoRender();
    return this;
};

GraphViewer.prototype.deselectNode = function(id) {
    if (this.selectedNodes.has(id)) {
        this.selectedNodes.delete(id);
        this._notifySelectionChange();
        this._autoRender();
    }
    return this;
};

GraphViewer.prototype.deselectAll = function() {
    if (this.selectedNodes.size > 0) {
        this.selectedNodes.clear();
        this._notifySelectionChange();
        this._autoRender();
    }
    return this;
};

GraphViewer.prototype.toggleSelection = function(id) {
    if (this.selectedNodes.has(id)) {
        this.deselectNode(id);
    } else {
        this.addToSelection(id);
    }
    return this;
};

GraphViewer.prototype.isSelected = function(id) {
    return this.selectedNodes.has(id);
};

GraphViewer.prototype.isNodeVisible = function(id) {
    var node = this.nodes.get(id);
    if (!node) return false;
    
    var screen = this._worldToScreen(node.x, node.y);
    var radius = this._getNodeRadius(node);
    var scaledRadius = radius * this.transform.scale;
    var fontSize = this.styles.nodeFontSize;
    var labelOffset = this.styles.nodeLabelOffset != null ? this.styles.nodeLabelOffset : 6;
    
    var margin = scaledRadius + fontSize * this.transform.scale + labelOffset * this.transform.scale;
    
    return screen.x >= -margin && screen.x <= this.displayWidth + margin &&
           screen.y >= -margin && screen.y <= this.displayHeight + margin;
};

GraphViewer.prototype.getSelectedNodes = function() {
    return Array.from(this.selectedNodes);
};

GraphViewer.prototype.setSelectionColor = function(color) {
    this.styles.nodeSelectionColor = color;
    this._autoRender();
    return this;
};

GraphViewer.prototype._notifySelectionChange = function() {
    var selected = this.getSelectedNodes();
    this.selectionChangeCallbacks.forEach(function(callback) {
        callback(selected);
    });
};

GraphViewer.prototype.centerOnNode = function(id) {
    var node = this.nodes.get(id);
    if (!node) return this;
    
    var centerX = this.displayWidth / 2;
    var centerY = this.displayHeight / 2;
    
    this.transform.offsetX = centerX - node.x * this.transform.scale;
    this.transform.offsetY = centerY - node.y * this.transform.scale;
    
    this.render();
    return this;
};

GraphViewer.prototype.setZoom = function(scale) {
    var centerX = this.displayWidth / 2;
    var centerY = this.displayHeight / 2;
    
    var worldX = (centerX - this.transform.offsetX) / this.transform.scale;
    var worldY = (centerY - this.transform.offsetY) / this.transform.scale;
    
    this.transform.scale = Math.max(this.minScale, Math.min(this.maxScale, scale));
    
    this.transform.offsetX = centerX - worldX * this.transform.scale;
    this.transform.offsetY = centerY - worldY * this.transform.scale;
    
    this.render();
    return this;
};

GraphViewer.prototype._autoRender = function() {
    if (this.autoRender) {
        this.render();
    }
};

GraphViewer.prototype.beginBatch = function() {
    this.autoRender = false;
    return this;
};

GraphViewer.prototype.endBatch = function() {
    this.autoRender = true;
    this.render();
    return this;
};

GraphViewer.prototype.setAutoRender = function(enabled) {
    this.autoRender = enabled;
    return this;
};

GraphViewer.prototype.render = function() {
    var ctx = this.ctx;
    var width = this.displayWidth;
    var height = this.displayHeight;
    
    ctx.fillStyle = this.styles.backgroundColor;
    ctx.fillRect(0, 0, width, height);
    
    this._renderGrid();
    this._renderEdges();
    this._renderNodes();
};

GraphViewer.prototype._renderGrid = function() {
    var ctx = this.ctx;
    var baseSpacing = this.styles.gridSpacing;
    var baseDotSize = this.styles.gridDotSize;
    var color = this.gridColorOverride || this.styles.gridColor;
    
    var skipFactor = 1;
    if (this.transform.scale < 0.5) skipFactor = 2;
    if (this.transform.scale < 0.25) skipFactor = 4;
    if (this.transform.scale < 0.125) skipFactor = 8;
    if (this.transform.scale < 0.0625) skipFactor = 16;
    
    var spacing = baseSpacing * skipFactor;
    var dotSize = Math.max(1, baseDotSize * this.transform.scale);
    
    var startWorldX = (0 - this.transform.offsetX) / this.transform.scale;
    var startWorldY = (0 - this.transform.offsetY) / this.transform.scale;
    var endWorldX = (this.displayWidth - this.transform.offsetX) / this.transform.scale;
    var endWorldY = (this.displayHeight - this.transform.offsetY) / this.transform.scale;
    
    var startX = Math.floor(startWorldX / spacing) * spacing;
    var startY = Math.floor(startWorldY / spacing) * spacing;
    var endX = Math.ceil(endWorldX / spacing) * spacing;
    var endY = Math.ceil(endWorldY / spacing) * spacing;
    
    var maxDotsPerAxis = 150;
    var dotsX = Math.ceil((endX - startX) / spacing);
    var dotsY = Math.ceil((endY - startY) / spacing);
    
    if (dotsX > maxDotsPerAxis || dotsY > maxDotsPerAxis) {
        var extraSkip = Math.ceil(Math.max(dotsX, dotsY) / maxDotsPerAxis);
        return this._renderGridWithSkip(startX, startY, endX, endY, spacing * extraSkip, dotSize, color);
    }
    
    this._renderGridWithSkip(startX, startY, endX, endY, spacing, dotSize, color);
};

GraphViewer.prototype._renderGridWithSkip = function(startX, startY, endX, endY, spacing, dotSize, color) {
    var ctx = this.ctx;
    ctx.fillStyle = color;
    
    var halfSize = dotSize / 2;
    
    for (var x = startX; x <= endX; x += spacing) {
        for (var y = startY; y <= endY; y += spacing) {
            var screen = this._worldToScreen(x, y);
            ctx.fillRect(screen.x - halfSize, screen.y - halfSize, dotSize, dotSize);
        }
    }
};

GraphViewer.prototype._isLineInViewport = function(x1, y1, x2, y2) {
    var w = this.displayWidth;
    var h = this.displayHeight;
    
    if ((x1 >= 0 && x1 <= w && y1 >= 0 && y1 <= h) ||
        (x2 >= 0 && x2 <= w && y2 >= 0 && y2 <= h)) {
        return true;
    }
    
    if ((x1 < 0 && x2 < 0) || (x1 > w && x2 > w) ||
        (y1 < 0 && y2 < 0) || (y1 > h && y2 > h)) {
        return false;
    }
    
    return true;
};

GraphViewer.prototype._renderEdges = function() {
    var ctx = this.ctx;
    var arrowSize = this.styles.edgeArrowSize;
    var w = this.displayWidth;
    var h = this.displayHeight;
    
    for (var i = 0; i < this.edges.length; i++) {
        var edge = this.edges[i];
        var fromNode = this.nodes.get(edge.from);
        var toNode = this.nodes.get(edge.to);
        
        if (!fromNode || !toNode) continue;
        
        var from = this._worldToScreen(fromNode.x, fromNode.y);
        var to = this._worldToScreen(toNode.x, toNode.y);
        
        if (!this._isLineInViewport(from.x, from.y, to.x, to.y)) continue;
        
        var dx = to.x - from.x;
        var dy = to.y - from.y;
        var length = Math.sqrt(dx * dx + dy * dy);
        
        if (length === 0) continue;
        
        var ux = dx / length;
        var uy = dy / length;
        
        var fromRadius = this._getNodeRadius(fromNode) * this.transform.scale;
        var toRadius = this._getNodeRadius(toNode) * this.transform.scale;
        var startX = from.x + ux * fromRadius;
        var startY = from.y + uy * fromRadius;
        var endX = to.x - ux * toRadius;
        var endY = to.y - uy * toRadius;
        
        var edgeColor = edge.color || this.styles.edgeColor;
        var edgeWidth = this.styles.edgeWidth;
        
        ctx.strokeStyle = edgeColor;
        ctx.lineWidth = edgeWidth;
        ctx.beginPath();
        ctx.moveTo(startX, startY);
        ctx.lineTo(endX, endY);
        ctx.stroke();
        
        var scaledArrow = arrowSize * this.transform.scale;
        var arrowAngle = Math.PI / 6;
        
        ctx.fillStyle = edgeColor;
        ctx.beginPath();
        ctx.moveTo(endX, endY);
        ctx.lineTo(
            endX - scaledArrow * Math.cos(Math.atan2(dy, dx) - arrowAngle),
            endY - scaledArrow * Math.sin(Math.atan2(dy, dx) - arrowAngle)
        );
        ctx.lineTo(
            endX - scaledArrow * Math.cos(Math.atan2(dy, dx) + arrowAngle),
            endY - scaledArrow * Math.sin(Math.atan2(dy, dx) + arrowAngle)
        );
        ctx.closePath();
        ctx.fill();
        
        if (edge.label) {
            var midX = (startX + endX) / 2;
            var midY = (startY + endY) / 2;
            
            ctx.fillStyle = edgeColor;
            ctx.font = (12 * this.transform.scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'bottom';
            ctx.fillText(edge.label, midX, midY - 5);
        }
    }
};

GraphViewer.prototype._renderNodes = function() {
    var ctx = this.ctx;
    var strokeWidth = this.styles.nodeStrokeWidth;
    var fontSize = this.styles.nodeFontSize;
    var labelOffset = this.styles.nodeLabelOffset != null ? this.styles.nodeLabelOffset : 6;
    var self = this;
    var w = this.displayWidth;
    var h = this.displayHeight;
    
    this.nodes.forEach(function(node, id) {
        var screen = self._worldToScreen(node.x, node.y);
        var radius = self._getNodeRadius(node);
        var scaledRadius = radius * self.transform.scale;
        
        var margin = scaledRadius + fontSize * self.transform.scale + labelOffset * self.transform.scale;
        if (screen.x < -margin || screen.x > w + margin ||
            screen.y < -margin || screen.y > h + margin) {
            return;
        }
        
        var isSelected = self.selectedNodes.has(id);
        
        var fillColor = isSelected 
            ? self.styles.nodeSelectionColor 
            : (node.color || self.styles.nodeFill);
        var strokeColor = node.strokeColor || self.styles.nodeStroke;
        
        ctx.beginPath();
        ctx.arc(screen.x, screen.y, scaledRadius, 0, Math.PI * 2);
        
        ctx.fillStyle = fillColor;
        ctx.fill();
        
        if (strokeWidth > 0) {
            ctx.strokeStyle = strokeColor;
            ctx.lineWidth = strokeWidth * self.transform.scale;
            ctx.stroke();
        }
        
        if (node.label) {
            ctx.fillStyle = self.styles.nodeLabelColor || self.styles.nodeStroke;
            ctx.font = (fontSize * self.transform.scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'top';
            var labelY = screen.y + scaledRadius + labelOffset * self.transform.scale;
            ctx.fillText(node.label, screen.x, labelY);
        }
    });
};

GraphViewer.prototype.save = function() {
    var nodes = [];
    var spacing = this.styles.gridSpacing;
    
    this.nodes.forEach(function(node, id) {
        nodes.push({
            id: id,
            gridX: Math.round(node.x / spacing),
            gridY: Math.round(node.y / spacing),
            label: node.label,
            color: node.color,
            strokeColor: node.strokeColor,
            radius: node.radius,
            data: node.data
        });
    });
    
    var edges = [];
    for (var i = 0; i < this.edges.length; i++) {
        var edge = this.edges[i];
        edges.push({
            from: edge.from,
            to: edge.to,
            color: edge.color,
            label: edge.label
        });
    }
    
    return {
        nodes: nodes,
        edges: edges
    };
};

GraphViewer.prototype.saveJSON = function() {
    return JSON.stringify(this.save());
};

GraphViewer.prototype.load = function(data) {
    var prevAutoRender = this.autoRender;
    this.autoRender = false;
    
    this.nodes.clear();
    this.edges = [];
    this.selectedNodes.clear();
    
    if (data.nodes) {
        for (var i = 0; i < data.nodes.length; i++) {
            var n = data.nodes[i];
            this.addNode(n.id, n.gridX, n.gridY, {
                label: n.label,
                color: n.color,
                strokeColor: n.strokeColor,
                radius: n.radius,
                data: n.data
            });
        }
    }
    
    if (data.edges) {
        for (var j = 0; j < data.edges.length; j++) {
            var e = data.edges[j];
            this.addEdge(e.from, e.to, {
                color: e.color,
                label: e.label
            });
        }
    }
    
    this.autoRender = prevAutoRender;
    this._autoRender();
    return this;
};

GraphViewer.prototype.loadJSON = function(json) {
    return this.load(JSON.parse(json));
};

GraphViewer.prototype.clear = function() {
    this.nodes.clear();
    this.edges = [];
    this.selectedNodes.clear();
    this._autoRender();
    return this;
};

GraphViewer.prototype.destroy = function() {
    if (this._resizeHandler) {
        window.removeEventListener('resize', this._resizeHandler);
    }
    if (this.canvas && this.canvas.parentNode) {
        this.canvas.parentNode.removeChild(this.canvas);
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = GraphViewer;
}
