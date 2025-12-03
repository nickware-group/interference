
function getChartOptions(size = 0) {
    return {
        scales: {
            yAxes: [{
                ticks: {
                    min: 0,
                    max: size
                }
            }],
            xAxes: [{
                ticks: {
                    min: 0,
                    max: size
                }
            }]
        },
        legend: {
            display: false
        },
        animation: {
            duration: 0,
            animateScale: false,
            animateRotate: false
        },
        plugins: {
            zoom: {
                // pan: {
                //     enabled: true,
                //     mode: 'xy',
                //
                //
                //     onPan: function({chart}) { console.log(`I'm panning!!!`); },
                //     // Function called once panning is completed
                //     onPanComplete: function({chart}) { console.log(`I was panned!!!`); }
                // },
                zoom: {
                    enabled: true,
                    mode: 'xy',
                    // onZoom: function({chart}) { console.log(`I'm zooming!!!`); },
                    // // Function called once zooming is completed
                    // onZoomComplete: function({chart}) { console.log(`I was zoomed!!!`); }
                }
            }
        },
        tooltips: {
            // Disable the on-canvas tooltip
            enabled: false,
            callbacks: {
                label: function(tooltipItem, data) {
                    return data['datasets'][tooltipItem['datasetIndex']]['data'][tooltipItem['index']];
                }
            },

            custom: function(tooltipModel) {
                // Tooltip Element
                let tooltipEl = document.getElementById('chartjs-tooltip');

                // Create element on first render
                if (!tooltipEl) {
                    tooltipEl = document.createElement('div');
                    tooltipEl.id = 'chartjs-tooltip';
                    tooltipEl.innerHTML = '<div></div>';
                    document.body.appendChild(tooltipEl);
                }

                // Hide if no tooltip
                if (tooltipModel.opacity === 0) {
                    tooltipEl.style.opacity = 0;
                    return;
                }

                // Set caret Position
                tooltipEl.classList.remove('above', 'below', 'no-transform');
                if (tooltipModel.yAlign) {
                    tooltipEl.classList.add(tooltipModel.yAlign);
                } else {
                    tooltipEl.classList.add('no-transform');
                }

                function getBody(bodyItem) {
                    return bodyItem.lines;
                }

                // Set Text
                if (tooltipModel.body) {
                    let bodyLines = tooltipModel.body.map(getBody);

                    let style = 'background: #171717AA';
                    style += '; color: #f3f3f3';
                    style += '; padding: 5px;';
                    style += '; border-radius: 8px;';
                    let innerHtml = '<div style=\"'+style+'\">';

                    bodyLines.forEach(function(body, i) {
                        let data = "x: "+Math.floor(body[i].x*1000)/1000+ ", y: "+Math.floor(body[i].y*1000)/1000;
                        innerHtml += '<div>'+body[i].type+'</div>';
                        innerHtml += '<div>'+data+'</div>';
                    });
                    innerHtml += '</div>';

                    let tableRoot = tooltipEl.querySelector('div');
                    tableRoot.innerHTML = innerHtml;
                }

                // `this` will be the overall tooltip
                let position = this._chart.canvas.getBoundingClientRect();

                // Display, position, and set styles for font
                tooltipEl.style.opacity = 1;
                tooltipEl.style.position = 'absolute';
                tooltipEl.style.left = position.left + window.pageXOffset + tooltipModel.caretX + 'px';
                tooltipEl.style.top = position.top + window.pageYOffset + tooltipModel.caretY + 'px';
                tooltipEl.style.fontFamily = tooltipModel._bodyFontFamily;
                tooltipEl.style.fontSize = tooltipModel.bodyFontSize + 'px';
                tooltipEl.style.fontStyle = tooltipModel._bodyFontStyle;
                tooltipEl.style.padding = tooltipModel.yPadding + 'px ' + tooltipModel.xPadding + 'px';
                tooltipEl.style.pointerEvents = 'none';
            }
        },
        onClick: function(e) {
            // eslint-disable-next-line no-alert
            console.log(e.type);
        }
    };
}

function doShowNeuronModel(size, spos, rpos0, rpos = [], rposf = [], frame = -1) {
    //console.log("model", spos, rpos);

    let data = {
        datasets: [{
            data: spos,
            backgroundColor: 'rgba(255,143,109,0.88)'
        }, {
            data: rpos0,
            backgroundColor: 'rgba(189,189,189,0.91)'
        }, {
            data: rpos,
            backgroundColor: 'rgba(11,158,250,0.91)'
        }, {
            data: rposf,
            backgroundColor: 'rgba(134,11,250,0.91)'
        }]
    };

    if (frame === -1) {
        let e = document.getElementById("NMC"+(FManager.getCurrentFrameID()+1));
        FManager.getCurrentFrame().doInitNeuronModel(e, data, size);
    } else {
        let e = document.getElementById("NMC"+(frame+1));
        FManager.getFrame(frame).doInitNeuronModel(e, data, size);
    }
}

function doClearNeuronModel(frame) {
    let e = document.getElementById("NMC"+(frame+1));
    FManager.getFrame(frame).doClearNeuronModel(e);
}

function doUpdateModelData(size, spos, rpos0, rpos = [], rposf = []) {
    //let e = document.getElementById("NMC");
    let data = {
        datasets: [{
            data: spos,
            backgroundColor: 'rgba(255,143,109,0.88)'
        }, {
            data: rpos0,
            backgroundColor: 'rgba(189,189,189,0.91)'
        }, {
            data: rpos,
            backgroundColor: 'rgba(11,158,250,0.91)'
        }, {
            data: rposf,
            backgroundColor: 'rgba(134,11,250,0.91)'
        }]
    };

    if (frame === -1)
        FManager.getCurrentFrame().doUpdateNeuronModel(data, getChartOptions(size));
    else
        FManager.getFrame(frame).doUpdateNeuronModel(data, getChartOptions(size));
}

function doUpdateModel(frame, current) {
    let spos = [];
    let rpos = [];

    for (let i = 0; i < FManager.getFrame(frame).getData().neuron_list[current].synapses.length; i++) {
        if (FManager.getFrame(frame).getData().neuron_list[current].synapses[i].type === "cluster") {
            let pos = doComputeClusterPosition(FManager.getFrame(frame).getData().neuron_list[current].synapses[i].position,
                FManager.getFrame(frame).getData().neuron_list[current].synapses[i].radius,
                FManager.getFrame(frame).getData().neuron_list[current].input_signals.length,
                4,
                "Synapse "+(i+1)+" (cluster)");
            spos = spos.concat(pos);
        } else {
            spos.push({x: FManager.getFrame(frame).getData().neuron_list[current].synapses[i].position[0],
                y: FManager.getFrame(frame).getData().neuron_list[current].synapses[i].position[1],
                r: 4,
                type: "Synapse "+(i+1)});
        }
    }

    for (let i = 0; i < FManager.getFrame(frame).getData().neuron_list[current].receptors.length; i++) {
        if (FManager.getFrame(frame).getData().neuron_list[current].receptors[i].type === "cluster") {
            let pos = doComputeClusterPosition(FManager.getFrame(frame).getData().neuron_list[current].receptors[i].position,
                FManager.getFrame(frame).getData().neuron_list[current].receptors[i].radius,
                FManager.getFrame(frame).getData().neuron_list[current].receptors[i].count,
                2,
                "Receptor "+(i+1)+" (cluster)");
            rpos = rpos.concat(pos);
        } else {
            rpos.push({x: FManager.getFrame(frame).getData().neuron_list[current].receptors[i].position[0],
                y: FManager.getFrame(frame).getData().neuron_list[current].receptors[i].position[1],
                r: 2,
                type: "Default receptor "+(i+1)});
        }
    }

    doShowNeuronModel(FManager.getFrame(frame).getData().neuron_list[current].size, spos, rpos, frame);
}
