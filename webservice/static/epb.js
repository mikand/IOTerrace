var charts = {};
var lastUpdate = {};


function roundDatum(x) {
    return Math.round(x * roundingFactor) / roundingFactor;
}


function makeChart(id, port) {
    charts[port] = new Chart($(id)[0].getContext('2d'), graphOptions());
    lastUpdate[port] = 0;
    update();
}

function addData(chart, label, data) {
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset) => {
        dataset.data.push(data);
    });
}

function clearData(chart) {
    chart.data.labels.length = 0;
    chart.data.datasets.forEach((dataset) => {
        dataset.data.length = 0;
    });
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
    for (var port in charts) {
        $.getJSON("/get_readings/" + port + '/' + lastUpdate[port], function( msg ) {
            if (msg.status == "OK") {
                var data = msg.data;
                for (var i=0; i<data.length; i++) {
                    x = moment.unix(data[i].time)
                    v = roundDatum(data[i].value);
                    addData(charts[msg.port], x, {t:x.valueOf(), y:v});
                    if (data[i].time > lastUpdate[msg.port]) {
                        lastUpdate[msg.port] = data[i].time;
                    }
                }
                charts[msg.port].update();
            }
            else {
                notify("Errore: " + msg.status);
            }
        });
    }
}


$(document).ready(function() {

    // The original draw function for the line chart. This will be applied after we have drawn our highlight range (as a rectangle behind the line chart).
    var originalLineDraw = Chart.controllers.line.prototype.draw;
    // Extend the line chart, in order to override the draw function.
    Chart.helpers.extend(Chart.controllers.line.prototype, {
        draw : function() {
            var chart = this.chart;
            // Get the object that determines the region to highlight.
            var yHighlightRange = chart.config.data.yHighlightRange;

            // If the object exists.
            if (yHighlightRange !== undefined) {
                var ctx = chart.chart.ctx;

                var yRangeBegin = yHighlightRange.begin;
                var yRangeEnd = yHighlightRange.end;

                var xaxis = chart.scales['x-axis-0'];
                var yaxis = chart.scales['y-axis-0'];

                var yRangeBeginPixel = yaxis.getPixelForValue(yRangeBegin);
                var yRangeEndPixel = yaxis.getPixelForValue(yRangeEnd);

                ctx.save();

                // The fill style of the rectangle we are about to fill.
                ctx.fillStyle = 'rgba(0, 255, 0, 0.3)';
                // Fill the rectangle that represents the highlight region. The parameters are the closest-to-starting-point pixel's x-coordinate,
                // the closest-to-starting-point pixel's y-coordinate, the width of the rectangle in pixels, and the height of the rectangle in pixels, respectively.
                ctx.fillRect(xaxis.left, Math.min(yRangeBeginPixel, yRangeEndPixel), xaxis.right - xaxis.left, Math.max(yRangeBeginPixel, yRangeEndPixel) - Math.min(yRangeBeginPixel, yRangeEndPixel));

                ctx.restore();
            }

            // Apply the original draw function for the line chart.
            originalLineDraw.apply(this, arguments);
        }
    });

    setInterval(update, 1000);
});
