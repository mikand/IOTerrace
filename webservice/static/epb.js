var chart = null;
var lastUpdate = {};
var activeSensors = null;


function roundDatum(x) {
    return Math.round(x * roundingFactor) / roundingFactor;
}


function makeChart(id, sensors) {
    chart = new Chart($(id)[0].getContext('2d'), graphOptions());
    activeSensors = sensors;
    for (var s in sensors) {
        lastUpdate[s] = 0;
    }
    update();
}

function addDatum(chart, sensor, l, d) {
    chart.data.datasets[sensor].data.push({x:l, y:d});
    if (chart.data.datasets[sensor].data.length > num_labels) {
        chart.data.datasets[sensor].data.splice(0, 1);
    }
}

function notify(str) {
    $.notify(str, {
        // whether to hide the notification on click
        clickToHide: true,
        // whether to auto-hide the notification
        autoHide: true,
        // if autoHide, hide after milliseconds
        autoHideDelay: 60000,
        // show the arrow pointing at the element
        arrowShow: true,
        // arrow size in pixels
        arrowSize: 5,
        // default positions
        elementPosition: 'bottom left',
        globalPosition: 'bottom right',
        // default style
        style: 'bootstrap',
        // default class (string or [string])
        className: 'info',
        // show animation
        showAnimation: 'slideDown',
        // show animation duration
        showDuration: 400,
        // hide animation
        hideAnimation: 'slideUp',
        // hide animation duration
        hideDuration: 200,
        // padding between element and notification
        gap: 3
    });
}

function update() {
    for (var port in activeSensors) {
        $.getJSON("/get_readings/" + port + '/' + lastUpdate[port], function( msg ) {
            if (msg.status == "OK") {
                var data = msg.data;
                for (var i=0; i<data.length; i++) {
                    x = moment.unix(data[i].time)
                    v = roundDatum(data[i].value);
                    addDatum(chart, msg.port, x, v);
                    if (data[i].time > lastUpdate[msg.port]) {
                        lastUpdate[msg.port] = data[i].time;
                    }
                }
                chart.update();
            }
            else {
                notify("Error: " + msg.status);
            }
        });
    }
}


$(document).ready(function() {
    setInterval(update, 10000);
});
