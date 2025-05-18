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

    exec("g++ -std=c++11 parser/input.cpp -o parser.exe && parser.exe input.txt", (err, stdout, stderr) => {
        if (err) {
            console.error("Compilation or Execution Error:", err);
            console.error("stderr:", stderr);
            return res.status(500).send("Compilation or execution error");
        }

        if (!fs.existsSync("symbols.json")) {
            console.error("symbols.json not found!");
            return res.status(500).send("Symbol file not generated");
        }

        try {
            const symbols = JSON.parse(fs.readFileSync("symbols.json", "utf8"));
            res.json(symbols);
        } catch(parseErr) {
            console.error("Error parsing symbols.json:", parseErr);
            return res.status(500).send("Failed to parse symbols.json");
        }
    });
});

app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
