const path = require("path");
const express = require("express");
const fs = require("fs");
const { exec } = require("child_process");
const app = express();
const PORT = 3000;

app.use(express.json());
app.use(express.static("."));


// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, "public")));

app.post("/compile", (req, res) => {
    const code = req.body.code;
    fs.writeFileSync("input.txt", code);

    if (fs.existsSync("symbols.json")) {
    fs.unlinkSync("symbols.json"); // delete old symbol table
    }

    if (fs.existsSync("parse_tree.json")) fs.unlinkSync("parse_tree.json");


    exec("g++ -std=c++11 parser/input.cpp -o parser.exe && parser.exe input.txt", (err, stdout, stderr) => {
        if (err) {
            // Combine stdout and stderr so we catch parse errors (stdout) or compile errors (stderr)
            const output = (stdout + stderr).toString().trim() || "Compilation or execution error";
            console.error("Analyzer Error Output:", output);
            return res.status(400).json({ error: output });
        }


        if (!fs.existsSync("symbols.json")) {
            console.error("symbols.json not found!");
            return res.status(500).send("Symbol file not generated");
        }

        if (!fs.existsSync("parse_tree.json")) {
        res.status(500).send("parse_tree.json not found");
        return;
        }

        try {
            const symbolsRaw = JSON.parse(fs.readFileSync("symbols.json", "utf8"));
            const symbols = symbolsRaw.symbols || [];  // ✅ extract array safely

            const parseTree = JSON.parse(fs.readFileSync("parse_tree.json", "utf8"));

            res.json({ symbols, parseTree });
        } catch(parseErr) {
             console.error("Error parsing JSON outputs:", parseErr);
             return res.status(500).send("Failed to parse output JSON");
        }
        

    });
});

app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
