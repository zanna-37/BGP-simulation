mdrouter = document.querySelector("input[id='mdrouter']");
mdendpoint = document.querySelector("input[id='mdendpoint']");

var mode_val = 'R';

var num_interface = 1;

var from_node = undefined;
var to_node = undefined;
var new_link = undefined;

var chart = new NetChart({
    container: document.getElementById("netchart"),
    area: { height: 400 },
    data: { url: "/getNetZoomCharts" },
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
        onDoubleClick: modifyNodeNumbers,
        onClick: function (args) {
            var node_selected = args.clickNode;
            showNodeInfo(node_selected);
        },
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
        if (node.id.charAt(0) == 'R') {
            last_router = node.id.charAt(1);
        }
    }
    return last_router
}

function getLastEndpointId(nodes) {
    var last_endpoint = 0
    for (var node of nodes) {
        if (node.id.charAt(0) == 'E') {
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
        if (mode_val == 'E') {
            $('#ASnumberDiv').hide();
            $('#as_number').val(undefined);
        }
        else if (mode_val == 'R') {
            $('#ASnumberDiv').show();
        }
        $('#addDeviceModal').show();
        eventAddNode = event;
    }
    if (event.clickNode) {
        $("#nav-node-info").empty();
        $("#nav-node-BGPpeers").empty();
        $("#nav-node-routingTable").empty();
        $.ajax({
            url: "/removeNode",
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
                "id": event.clickNode.id,
            })
        });
        event.chart.removeData({ nodes: [{ id: event.clickNode.id }] });
    } else if (event.clickLink) {
        var link = event.clickLink;
        if (link.data.style == undefined) {
            link.data.style = { "fillColor": "#333" };
        }
        if (link.data.style.fillColor == "red") {
            $.ajax({
                url: "/addLink",
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
            link.data.style = { "fillColor": "#333" };
        } else {
            $.ajax({
                url: "/breakLink",
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
            link.data.style = { "fillColor": "red" };
        }
    }

    event.preventDefault();
}

$('#saveAddDeviceChanges').click(function () {
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
    if (mode_val == "R") {
        as_number = $("#as_number").val();
    }

    chart.addData({
        nodes: [{
            "id": mode_val == "R" ? mode_val + new_routers_Id : mode_val + new_endpoints_Id,
            "x": eventAddNode.chartX,
            "y": eventAddNode.chartY,
            "loaded": true,
            "style": {
                "label": mode_val == "R" ? "Router" + new_routers_Id : "Endpoint" + new_endpoints_Id,
                "image": mode_val == "R" ? "showGUI/router-icon.png" : "showGUI/endpoint-icon.png"
            },
            "extra": {
                "default_gateway": default_gateway,
                "networkCard": networkCard,
                "AS_number": mode_val == "R" ? as_number : undefined
            }
        }]
    });

    $.ajax({
        url: "/addNode",
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
            "id": mode_val == "R" ? mode_val + new_routers_Id : mode_val + new_endpoints_Id,
            "gateway": default_gateway,
            "asNumber": mode_val == "R" ? as_number : undefined,
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

    if (mode_val == 'R') {
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

        var link = { "id": "link_" + node.id + "-" + onode.id, "from": node.id, "to": onode.id, extra: { "from_interface": " ", "to_interface": " " } }
        chart.addData({ nodes: [], links: [link] });

        activateLink(node, onode, link);
    }
}

function activateLink(node, onode, link) {
    from_node = node;
    to_node = onode;
    new_link = link;

    $("#from_label").append(node.data.id);
    $("#to_label").append(onode.data.id);

    $("#addLinkModal").show();

    var from_interface_options = "";
    var to_interface_options = "";

    for (var i = 0; i < node.data.extra.networkCard.length; i++) {
        from_interface_options += "<option>" + node.data.extra.networkCard[i].interface + "   " + node.data.extra.networkCard[i].IP + "   " + node.data.extra.networkCard[i].netmask + "</option>";
    }
    $("#from_interface").append(from_interface_options);

    for (var i = 0; i < onode.data.extra.networkCard.length; i++) {
        to_interface_options += "<option>" + onode.data.extra.networkCard[i].interface + "   " + onode.data.extra.networkCard[i].IP + "   " + onode.data.extra.networkCard[i].netmask + "</option>";
    }
    $("#to_interface").append(to_interface_options);

}

$('#saveAddLinkChanges').click(function () {
    var from_interface_value = $("#from_interface").val();
    var from_interface = "";
    var i = 0;
    while (from_interface_value[i] != " ") {
        from_interface += from_interface_value[i];
        i++
    }

    var to_interface_value = $("#to_interface").val();
    var to_interface = "";
    var j = 0;
    while (to_interface_value[j] != " ") {
        to_interface += to_interface_value[j];
        j++
    }

    new_link.extra.from_interface = from_interface;
    new_link.extra.to_interface = to_interface;

    $.ajax({
        url: "/addLink",
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
            "from": from_node.data.id,
            "to": to_node.data.id,
            "from_interface": from_interface,
            "to_interface": to_interface
        })
    });

    $("#addLinkModal").hide();

    $("#from_label").html("");
    $("#to_label").html("");
    $("#from_label").append("From ");
    $("#to_label").append("To ");

    $('#from_interface').children().remove().end().append('<option selected>Select Interface</option>');
    $('#to_interface').children().remove().end().append('<option selected>Select Interface</option>');
});


mdrouter.addEventListener('click', function () {
    mode_val = "R";
});

mdendpoint.addEventListener('click', function () {
    mode_val = "E";
});

$("#send-packet").click(function () {
    $("#sendPacketModal").show();

    var nodes = chart.nodes();
    var nodes_options = "";

    var from_node;

    console.log(nodes);

    for (var i = 0; i < nodes.length; i++) {
        if (nodes[i].data.extra.AS_number == undefined) {
            nodes_options += "<option>" + nodes[i].id + "</option>";
        }
    }

    $("#send_from").append(nodes_options);
    $("#send_to").append(nodes_options);
});

$("#saveSendPacketChanges").click(function () {
    var from = $("#send_from").val();
    var to = $("#send_to").val();

    console.log(from);
    console.log(to);

    $.ajax({
        url: "/sendPacket",
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
            "send_from": from,
            "send_to": to,
        })
    });

    $("#sendPacketModal").hide();

    $('#send_from').children().remove().end().append('<option selected>Select Device</option>');
    $('#send_to').children().remove().end().append('<option selected>Select Device</option>');
});

$("#closeSendPacketModalCross").click(function () {
    $("#sendPacketModal").hide();
})

$("#closeSendPacketModalBtn").click(function () {
    $("#sendPacketModal").hide();
})

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

$("#closeAddLinkModalCross").click(function () {
    $("#addLinkModal").hide();
})

$("#closeAddLinkModalBtn").click(function () {
    $("#addLinkModal").hide();
})

function showNodeInfo(node) {
    $("#nav-node-info").empty();
    $("#nav-node-BGPpeers").empty();
    $("#nav-node-routingTable").empty();

    var node_id = "Node ID: " + node.id + "<br>";

    var node_ip = "IP Address: ";
    for (var i = 0; i < node.data.extra.networkCard.length; i++) {
        var netCard = node.data.extra.networkCard[i];
        node_ip += netCard.IP + "(" + netCard.interface + ")     ";
    }
    node_ip += "<br>";

    var node_type = "Node Type: ";
    node_type += node.id[0] == "R" ? "Router" : "Endpoint";

    var node_content_info = node_id + node_ip + node_type;

    $("#nav-node-info").append(node_content_info);

    if (node.data.extra.AS_number) {
        //var peer_ip_address = [];
        var bgp_body = "<tbody>";
        var bgp_body_rows = "";
        var num_bgp_peers;
        $.ajax({
            url: "/getBGPpeersInfo",
            dataType: 'json',
            method: "POST",
            contentType: 'application/json',
            crossDomain: false,
            async: false,
            headers: {
                "accept": "application/json",
                "Access-Control-Allow-Origin": "*",
            },
            data: JSON.stringify({
                "id": node.id
            }),
        }).done(function (response) {
            num_bgp_peers = response.BGPpeers.length;
            for (var i = 0; i < response.BGPpeers.length; i++) {
                bgp_body_rows += "<tr>" +
                    "<td>" + response.BGPpeers[i].ip_address + "</td>" +
                    "<td>" + response.BGPpeers[i].identifier + "</td>" +
                    "<td>" + response.BGPpeers[i].status + "</td>" +
                    "</tr>";
                console.log(bgp_body_rows)
            }
        });

        bgp_body += bgp_body_rows + "</tbody>";
        console.log(bgp_body);

        var bgp_head = "<thead>" + "<tr>" +
            "<th>" + "IP Address" + "</th>" +
            "<th>" + "Identifier" + "</th>" +
            "<th>" + "Status" + "</th>" +
            "</tr>" + "</thead>";


        var BGP_table = "<table class='table table-striped'>" + bgp_head + bgp_body + "</table>";

        var routing_body = "<tbody>";
        var routing_body_rows = "";
        $.ajax({
            url: "/getRoutingTable",
            dataType: 'json',
            method: "POST",
            contentType: 'application/json',
            crossDomain: false,
            async: false,
            headers: {
                "accept": "application/json",
                "Access-Control-Allow-Origin": "*",
            },
            data: JSON.stringify({
                "id": node.id
            }),
        }).done(function (response) {
            for (var i = 0; i < response.routingTable.length; i++) {

                var asPath = response.routingTable[i].asPath;
                if (asPath == undefined) {
                    asPath = "";
                }

                routing_body_rows += "<tr>" +
                    "<td>" + response.routingTable[i].destination + "</td>" +
                    "<td>" + response.routingTable[i].nexthop + "</td>" +
                    "<td>" + response.routingTable[i].interface + "</td>" +
                    "<td>" + asPath + "</td>" +
                    "</tr>";
            }
        });

        routing_body += routing_body_rows + "</tbody>";

        var routing_head = "<thead>" + "<tr>" +
            "<th>" + "Destination" + "</th>" +
            "<th>" + "Next Hop" + "</th>" +
            "<th>" + "Interface" + "</th>" +
            "<th>" + "AS Path" + "</th>" +
            "</tr>" + "</thead>"

        var routing_table = "<table class='table table-striped'>" + routing_head + routing_body + "</table>"

        console.log(routing_table);

        if (num_bgp_peers > 0) {
            $("#nav-node-BGPpeers").append(BGP_table);
            $("#nav-node-routingTable").append(routing_table);
        }
    }
}



