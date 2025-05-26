document.getElementById("compile-btn").addEventListener("click", async () => {
    const code = window.editor.getValue(); // Get code from Monac
    
    const spinner = document.getElementById("spinner");
    spinner.style.display = "inline-block"; // Show spinner
    const res = await fetch("/compile", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code })
    });

    spinner.style.display = "none"; // Hide spinner after response

    const errorBox = document.querySelector("#error-box") || (() => {
    const div = document.createElement("div");
    div.id = "error-box";
    div.className = "error";
    document.querySelector(".editor").appendChild(div);
    return div;
    })();

    if (!res.ok) {
        const { error } = await res.json();
        errorBox.textContent = error;
        errorBox.style.display = "block";
        return;
    }

    errorBox.style.display = "none"; // hide on success


    const data = await res.json(); // only run if JSON is expected

    const tableBody = document.querySelector("#symbol-table tbody");
    tableBody.innerHTML = "";

    data.symbols.forEach(sym => {
        const row = `<tr><td>${sym.name}</td><td>${sym.type}</td></tr>`;
        tableBody.innerHTML += row;
    });

    const treeData = data.parseTree;
    drawParseTree(treeData);

    function drawParseTree(treeData) {
    // Clear previous tree
    d3.select("#ast-visualization").selectAll("*").remove();

    const width = 1200;
    const height = 1000;

    const svg = d3.select("#ast-visualization")
        .append("svg")
        .attr("width", width)
        .attr("height", height)
        .call(d3.zoom().on("zoom", (event) => {
            svgGroup.attr("transform", event.transform);
        }));

        const svgGroup = svg.append("g").attr("transform", "translate(60,40)");

        const root = d3.hierarchy(treeData);
        const treeLayout = d3.tree().nodeSize([40, 140]); // x: horizontal spread, y: vertical depth

        treeLayout(root);

        // Links (edges)
        svgGroup.selectAll(".link")
            .data(root.links())
            .enter().append("path")
            .attr("class", "link")
            .attr("fill", "none")
            .attr("stroke", "#999")
            .attr("stroke-width", 1.5)
            .attr("d", d3.linkVertical()
                .x(d => d.x)
                .y(d => d.y));

        // Nodes (circles)
        const node = svgGroup.selectAll(".node")
            .data(root.descendants())
            .enter().append("g")
            .attr("class", "node")
            .attr("transform", d => `translate(${d.x},${d.y})`);

        node.append("circle")
            .attr("r", 6)
            .attr("fill", "#3399cc");

        // Labels above the node
        node.append("text")
            .attr("text-anchor", "middle")
            .attr("dy", "-0.8em")
            .style("font-size", "12px")
            .text(d => d.data.name);

            document.getElementById("download-btn").onclick = () => {
            const svgNode = document.querySelector("#ast-visualization svg");

            const serializer = new XMLSerializer();
            const source = serializer.serializeToString(svgNode);

            const svgBlob = new Blob([source], { type: "image/svg+xml;charset=utf-8" });
            const url = URL.createObjectURL(svgBlob);

            // Create a temporary link and trigger download
            const downloadLink = document.createElement("a");
            downloadLink.href = url;
            downloadLink.download = "parse_tree.svg";
            document.body.appendChild(downloadLink);
            downloadLink.click();
            document.body.removeChild(downloadLink);
            URL.revokeObjectURL(url);
            };
    }

});
