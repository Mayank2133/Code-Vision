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
    const container = d3.select("#ast-visualization");
    const width = container.node().clientWidth;
    const height = container.node().clientHeight;

    const svg = container.append("svg")
        .attr("width", width)
        .attr("height", height)
        .append("g")
        .attr("transform", `translate(${width / 2}, 40)`); // move root to top center

    // Build hierarchy
    const root = d3.hierarchy(treeData);
    const treeLayout = d3.tree().size([width - 100, height - 100]);
    treeLayout(root);

    // Links
    svg.selectAll(".link")
        .data(root.links())
        .enter().append("path")
        .attr("class", "link")
        .attr("fill", "none")
        .attr("stroke", "#333")
        .attr("stroke-width", 1.5)
        .attr("d", d3.linkVertical()
            .x(d => d.x - width / 2) // center horizontally
            .y(d => d.y));

    // Nodes
    const node = svg.selectAll(".node")
        .data(root.descendants())
        .enter().append("g")
        .attr("class", "node")
        .attr("transform", d => `translate(${d.x - width / 2},${d.y})`);

    node.append("circle")
        .attr("r", 5)
        .attr("fill", "#fff")
        .attr("stroke", "#333")
        .attr("stroke-width", 1.5);

    node.append("text")
        .attr("dy", -10) // move text above node to avoid overlap
        .attr("text-anchor", "middle")
        .style("font-family", "Arial, sans-serif")
        .style("font-size", "13px")
        .style("fill", "#222") // same color as stroke
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
