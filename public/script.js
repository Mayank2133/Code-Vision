document.getElementById("compile-btn").addEventListener("click", async () => {
    const code = document.getElementById("code").value;
    const res = await fetch("/compile", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code })
    });

    if (!res.ok) {
    const text = await res.text(); // read as plain text
    alert("Server error: " + text);
    return;
    }

    const data = await res.json(); // only run if JSON is expected

    const tableBody = document.querySelector("#symbol-table tbody");
    tableBody.innerHTML = "";

    data.symbols.forEach(sym => {
        const row = `<tr><td>${sym.name}</td><td>${sym.type}</td><td>-</td></tr>`;
        tableBody.innerHTML += row;
    });
});
