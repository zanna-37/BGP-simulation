mdrouter = document.querySelector("input[id='mdrouter']");
mdendpoint = document.querySelector("input[id='mdendpoint']");

var mode_val = 'r';

var num_interface = 1;

var chart = new NetChart({
    container: document.getElementById("netchart"),
    area: { height: 400 },
    data: { url: "http://localhost:9080/" },
    info: {
        enabled: true,
        nodeContentsFunction: function (itemData, item) {
            var nodeInfo = itemData.extra;
            var infoContent = "";
            if (nodeInfo.AS_number != undefined) {
                infoContent += "AS Number: " + nodeInfo.AS_number + "<br>";
            }
            if (nodeInfo.default_gateway != undefined) {
                infoContent += "Default Gateway: " + nodeInfo.default_gateway + "<br>";
            }
            if (nodeInfo.networkCard != undefined) {
                infoContent += "Network Card:";
                infoContent += "<ul>"
                for (var i = 0; i < nodeInfo.networkCard.length; i++) {
                    var netCardItem = nodeInfo.networkCard[i];
                    infoContent += "<li> interface: " + netCardItem.interface + "</li>";
                    infoContent += "<li> IP: " + netCardItem.IP + "</li>";
                    infoContent += "<li> netmask: " + netCardItem.netmask + "</li>";
                }
                infoContent += "</ul>";
            }

            return "<div style='margin:auto; width:200px; height:100%; padding': 10px;>" +
                infoContent + "</div>";
        }
    },
    events: {
        onClick: modifyNodeNumbers,
        onPointerUp: function (e, args) {
            if (args.clickNode) {
                var node = args.clickNode;
                var onodes = getOverlappingNodes(node);
                connectNodes(node, onodes);
            }
        }
    }
});

function getLastRouterdId(nodes) {
    var last_router = 0;
    for (var node of nodes) {
        if (node.id.charAt(0) == 'r') {
            last_router = node.id.charAt(1);
        }
    }
    return last_router
}

function getLastEndpointId(nodes) {
    var last_endpoint = 0
    for (var node of nodes) {
        if (node.id.charAt(0) == 'e') {
            last_endpoint = node.id.charAt(1);
        }
    }
    return last_endpoint
}

var eventAddNode = undefined;
var new_routers_Id = 0;
var new_endpoints_Id = 0;

function modifyNodeNumbers(event) {
    new_routers_Id = parseInt(getLastRouterdId(chart.nodes())) + 1;
    new_endpoints_Id = parseInt(getLastEndpointId(chart.nodes())) + 1;

    if (!event.clickNode && !event.clickLink) {//test the click was on empty space
        if (mode_val == 'e') {
            $('#ASnumberDiv').hide();
            $('#as_number').val(undefined);
        }
        else if (mode_val == 'r') {
            $('#ASnumberDiv').show();
        }
        $('#addDeviceModal').show();
        eventAddNode = event;
    }
    if (event.clickNode) {
        event.chart.removeData({ nodes: [{ id: event.clickNode.id }] });
    } else if (event.clickLink) {
        var link = event.clickLink;
        $.ajax({
            url: "http://localhost:9080/breakLink",
            dataType: 'json',
            method: "POST",
            contentType: 'application/json',
            crossDomain: true,
            async: true,
            headers: {
                "accept": "application/json",
                "Access-Control-Allow-Origin": "*",
            },
            data: JSON.stringify({
                "from": link.data.from,
                "to": link.data.to,
                "from_interface": link.data.extra.from_interface,
                "to_interface": link.data.extra.to_interface
            })
        });
        event.chart.removeData({ links: [{ id: event.clickLink.id }] });
    }

    event.preventDefault();
}

$('#saveChanges').click(function () {
    var default_gateway = $("#default_gateway").val();
    var networkCard = [];
    for (var i = 1; i <= num_interface; i++) {
        networkCard.push({
            "interface": $('#interface-label-' + i).val(),
            "IP": $('#interface-IP-' + i).val(),
            "netmask": $('#interface-netmask-' + i).val()
        });
    }
    var as_number = undefined;
    if (mode_val == "r") {
        as_number = $("#as_number").val();
    }

    chart.addData({
        nodes: [{
            "id": mode_val == "r" ? mode_val + new_routers_Id : mode_val + new_endpoints_Id,
            "x": eventAddNode.chartX,
            "y": eventAddNode.chartY,
            "loaded": true,
            "style": {
                "label": mode_val == "r" ? "Router" + new_routers_Id : "Endpoint" + new_endpoints_Id,
                "image": mode_val == "r" ? "showGUI/router-icon.png" : "showGUI/endpoint-icon.png"
            },
            "extra": {
                "default_gateway": default_gateway,
                "networkCard": networkCard,
                "AS_number": mode_val == "r" ? as_number : undefined
            }
        }]
    });

    $.ajax({
        url: "http://localhost:9080/addNode",
        dataType: 'json',
        method: "POST",
        contentType: 'application/json',
        crossDomain: true,
        async: true,
        headers: {
            "accept": "application/json",
            "Access-Control-Allow-Origin": "*",
        },
        data: JSON.stringify({
            "id": mode_val == "r" ? mode_val + new_routers_Id : mode_val + new_endpoints_Id,
            "gateway": default_gateway,
            "asNumber": mode_val == "r" ? as_number : undefined,
            "networkCards": networkCard
        })
    });

    $('#addDeviceModal').hide();

    if (num_interface > 1) {
        for (var i = 2; i <= num_interface; i++) {
            $("#interface-label-" + i).remove();
            $("#interface-IP-" + i).remove();
            $("#interface-netmask-" + i).remove();
        }
        num_interface = 1;
    }

    $("#default_gateway").val(undefined);
    for (var i = 1; i <= num_interface; i++) {
        $('#interface-label-' + i).val(undefined);
        $('#interface-IP-' + i).val(undefined);
        $('#interface-netmask-' + i).val(undefined);
    }

    if (mode_val == 'r') {
        $("#as_number").val(undefined);
    }

});

function getOverlappingNodes(node) {
    if (!node) return;

    var found = [];
    var dim = chart.getNodeDimensions(node);

    var x = x1 = dim.x;
    var y = y1 = dim.y;
    var radius = dim.radius;

    //get all nodes:
    var nodes = chart.nodes();
    for (var i = 0; i < nodes.length; i++) {
        var obj = nodes[i];
        //skip dragged node itself.
        if (obj.id === node.id) {
            continue;
        }
        var odim = chart.getNodeDimensions(obj);
        var x0 = odim.x;
        var y0 = odim.y;

        var m = Math.sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) < radius;
        if (m) {
            found.push(obj);
        }
    }
    return found;
}

function connectNodes(node, onodes) {
    for (var i = 0; i < onodes.length; i++) {
        var onode = onodes[i];

        var link = { "id": "link_" + node.id + "-" + onode.id, "from": node.id, "to": onode.id, style: { "toDecoration": "arrow" } }
        chart.addData({ nodes: [], links: [link] });
    }
}

mdrouter.addEventListener('click', function () {
    mode_val = "r";
});

mdendpoint.addEventListener('click', function () {
    mode_val = "e";
});

$("#add-interface-btn").click(function () {
    num_interface += 1;
    $("#interfaces-label").append("<input type='text' class='form-control' style='margin-bottom: 10px;' id='interface-label-" + num_interface + "'>");
    $("#interfaces-IP").append("<input type='text' class='form-control' style='margin-bottom: 10px;' id='interface-IP-" + num_interface + "'>");
    $("#interfaces-netmask").append("<input type='text' class='form-control' style='margin-bottom: 10px;' id='interface-netmask-" + num_interface + "'>");
});

$("#closeAddDeviceModalCross").click(function () {
    $("#addDeviceModal").hide();
})

$("#closeAddDeviceModalBtn").click(function () {
    $("#addDeviceModal").hide();
})

