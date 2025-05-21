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
});
