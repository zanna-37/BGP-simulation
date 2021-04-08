mdrouter = document.querySelector("input[id='mdrouter']");
mdendpoint = document.querySelector("input[id='mdendpoint']");

var mode_val = 'r';

var chart = new NetChart({
    container: document.getElementById("netchart"),
    area: { height: 400},
    data: { url: "http://localhost:9080/" },
    events: {
        onClick: modifyNodeNumbers,
        onPointerUp: function(e, args) {
            if(args.clickNode) {
                var node = args.clickNode;
                var onodes = getOverlappingNodes(node);
                connectNodes(node, onodes);
            }
        }
    }
});

function getRoutersTot(nodes){
    var tot_routers = 0
    for (var node of nodes){
        if (node.id.charAt(0) == 'r'){
            tot_routers += 1
        }
    }
    return tot_routers
}

function getEndpointsTot(nodes){
    var tot_endpoints = 0
    for (var node of nodes){
        if (node.id.charAt(0) == 'e'){
            tot_endpoints += 1
        }
    }
    return tot_endpoints
}

function modifyNodeNumbers(event){
    var new_routers_Id = getRoutersTot(chart.nodes()) + 1;
    var new_endpoints_Id = getEndpointsTot(chart.nodes()) + 1; 

    if (!event.clickNode && !event.clickLink){//test the click was on empty space
        chart.addData({nodes:[{"id":mode_val == "r" ? mode_val + new_routers_Id : mode_val + new_endpoints_Id, 
                                "x":event.chartX, 
                                "y":event.chartY,
                                "loaded":true,
                                "style": {
                                    "label": mode_val == "r" ? "Router"+ new_routers_Id : "Endpoint"+ new_endpoints_Id,
                                    "image": mode_val == "r" ? "./img/router-icon.png" : "./img/endpoint-icon.png"
                                }
                                }]});
    }
    if (event.clickNode){
            event.chart.removeData({nodes:[{id:event.clickNode.id}]});
    }else if (event.clickLink){
        event.chart.removeData({links:[{id:event.clickLink.id}]});
    }
    event.preventDefault();
}

function getOverlappingNodes(node) {
    if(!node) return;

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
        if(obj.id === node.id) {
            continue;
        }
        var odim = chart.getNodeDimensions(obj);
        var x0 = odim.x;
        var y0 = odim.y;

        var m = Math.sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0)) < radius;
        if(m) {
            found.push(obj);
        }
    }
    return found;
}

function connectNodes(node, onodes) {
    for (var i = 0; i < onodes.length; i++) {
        var onode = onodes[i];

        var link = {"id": "link_" + node.id + "-" + onode.id,"from": node.id, "to": onode.id, style: {"toDecoration": "arrow"}}
        chart.addData({nodes:[],links: [link]});
    }
}

mdrouter.addEventListener('click', function() {
    mode_val = "r";
});

mdendpoint.addEventListener('click', function() {
    mode_val = "e";
});

