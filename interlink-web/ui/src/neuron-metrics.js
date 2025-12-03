let NeuronMetricsChart = null;
let NeuronMetricsRanges = [];
let NeuronMetricsTotals = {};

let NeuronMetricsOutputStepSize = 10000;

function doShowMetrics(output_data) {
    //console.log("model", spos, rpos);

    let e = document.getElementById("IMC");
    if (NeuronMetricsChart) NeuronMetricsChart.destroy();
    e.innerHTML = "";

    let chart_data = [];
    let labels = [];

    for (let i = 0; i < output_data.length; i++) {
        chart_data.push(parseFloat(output_data[i]));
        labels.push(i);
    }

    // for (let i = 1; i < 100; i++) {
    //     chart_data.push(1/i);
    //     labels.push(i);
    // }

    console.log("chart_data", chart_data, labels);
    console.log(randomScalingFactor());

    let data = {
        labels: labels,
        datasets: [{
            data: chart_data,
            backgroundColor: 'rgba(255,143,109,0.88)'
        }]
    };

    NeuronMetricsChart = new Chart(e, {
        type: 'bar',
        data: data,
        options: {
            responsive: true,
            maintainAspectRatio: false,
            tooltips: {
                mode: 'index',
                intersect: false,
            },
            scales: {
                xAxes: [{
                    display: false,
                    scaleLabel: {
                        display: false,
                        labelString: 'Time'
                    }
                }],
                yAxes: [{
                    display: true,
                    scaleLabel: {
                        display: false,
                        labelString: 'Value'
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
            hover: {
                animationDuration: 0 // duration of animations when hovering an item
            },
            responsiveAnimationDuration: 0 // animation duration after a resize
        }
        //options: getChartOptions(size),
    });

    // if (NeuronModelChart === null) {
    //
    // } else {
    //     console.log("updated");
    //     NeuronModelChart.config.data = data;
    //     NeuronModelChart.update('none');
    //     NeuronModelChart.config.options = options;
    //     NeuronModelChart.update('none');
    // }
}

function doInitMetricsShowRanges(element = "") {
    let current_selected = element;
    if (current_selected === "") current_selected = FManager.getInterlinkFrame().getLastSelectedElement();
    if (current_selected === "") return;

    let size = NeuronMetricsTotals[current_selected][facefull.Lists["NMDL"].getState()];
    console.log("metrics size", size)
    facefull.Comboboxes["MRCB"].doClear();
    NeuronMetricsRanges = [];

    if (size === 0) {
        facefull.Comboboxes["MRCB"].doAddItem(0);
        NeuronMetricsRanges.push([0, 0]);
    } else {
        let steps = size / NeuronMetricsOutputStepSize;
        let i;

        for (i = 0; i+NeuronMetricsOutputStepSize < size; i += NeuronMetricsOutputStepSize) {
            facefull.Comboboxes["MRCB"].doAddItem(i + " - " + (i+NeuronMetricsOutputStepSize));
            NeuronMetricsRanges.push([i, i+NeuronMetricsOutputStepSize]);
        }

        //i -= NeuronMetricsOutputStepSize;
        if (i < size) {
            let etab = document.createElement("div");
            facefull.Comboboxes["MRCB"].doAddItem(i + " - " + size);
            NeuronMetricsRanges.push([i, size]);
        }
    }

    facefull.Comboboxes["MRCB"].onChangeState = function(num) {
        let range = NeuronMetricsRanges[num];
        facefull.doEventSend("doLoadMetrics", current_selected+"|"+facefull.Lists["NMDL"].getState()+"|"+range[0]+"|"+range[1]);
        if (element === "") doCreateParameterList(FManager.getInterlinkFrame().getLastSelectedElement());
    };

    facefull.Comboboxes["MRCB"].setState(0);

    console.log(NeuronMetricsRanges);
}

function doAddMetricsToList(info) {
    let data = JSON.parse(info);
    // console.log(data);

    for (let i = 0; i < data.length; i++) {
        // console.log(data[i])
        if (NeuronMetricsTotals[data[i]["name"]] === null || NeuronMetricsTotals[data[i]["name"]] === undefined)
            NeuronMetricsTotals[data[i]["name"]] = [];

        NeuronMetricsTotals[data[i]["name"]].push(data[i]["total_time"]);
    }

    facefull.Lists["NMDL"].doAdd(["Data scope "+(facefull.Lists["NMDL"].getLength()+1)]);
    facefull.Scrollboxes["NMSB"].doUpdateScrollbar();
}

function doDeleteAllMetrics() {
    facefull.Lists["NMDL"].doClear();
    facefull.doEventSend("doClearMetricsData");
    facefull.Scrollboxes["NMSB"].doUpdateScrollbar();
    for (let n in FManager.getInterlinkFrame().getData().neuron_list) {
        for (let i = 0; i < FManager.getInterlinkFrame().getData().neuron_list[n].receptors.length; i++) {
            FManager.getInterlinkFrame().getData().neuron_list[n].receptors[i].data_scopes = [];
        }
    }
}
