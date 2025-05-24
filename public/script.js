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
        const row = `<tr><td>${sym.name}</td><td>${sym.type}</td><td>-</td></tr>`;
        tableBody.innerHTML += row;
    });

    const treeData = data.parseTree;
    drawParseTree(treeData);

    function drawParseTree(treeData) {
    // Clear any previous tree
    d3.select("#ast-visualization").selectAll("*").remove();

    // Set dimensions
    const width = 600, height = 400;
    const svg = d3.select("#ast-visualization")
                    .append("svg")
                    .attr("width", width)
                    .attr("height", height)
                    .append("g")
                    .attr("transform", "translate(40,0)");

    // Create a d3 hierarchy and layout
    const root = d3.hierarchy(treeData);
    const treeLayout = d3.tree().size([height, width - 80]);
    treeLayout(root);

    // Draw links (edges)
    svg.selectAll(".link")
        .data(root.links())
        .enter().append("path")
        .attr("class", "link")
        .attr("d", d3.linkHorizontal()
                    .x(d => d.y)
                    .y(d => d.x));

    // Draw nodes (vertices)
    const node = svg.selectAll(".node")
                    .data(root.descendants())
                    .enter().append("g")
                    .attr("class", "node")
                    .attr("transform", d => `translate(${d.y},${d.x})`);

    node.append("circle")
        .attr("r", 4);
    node.append("text")
        .attr("dy", 3)
        .attr("x", d => d.children ? -8 : 8)
        .style("text-anchor", d => d.children ? "end" : "start")
        .text(d => d.data.name);
    }

});
