# 🧠 SyntaxScope - Code Vision

## 📌 Project Overview
A full-stack web application that allows users to input C++ code, which is then processed through a custom-built C++ lexical and syntax analyzer. The application returns a symbol table and (upcoming) abstract syntax tree (AST) for visualization in the browser. Built using **Node.js**, **Express**, and **C++**.

![Code editor](./screenshots/Screenshot%202025-05-26%20190746.png)  
![Symbol Table Output](/screenshots/Screenshot%202025-05-26%20190759.png)  
![Compiler JSON Output](/screenshots/Screenshot%202025-05-26%20195222.png)

---

## 🚀 Key Features
- **Frontend Code Editor** (via HTML textarea)
- **Node.js/Express Backend API**
- **C++ Lexical & Syntax Analyzer**
- **Symbol Table Output in JSON**
- **Dynamic Symbol Table Viewer**
- **Error Handling for Compilation/Parsing**
- **Scalable Architecture for AST Support**

---

## 📂 Project Structure
Code-Vision/
├── public/
│ ├── index.html # Code editor UI
│ ├── script.js # Fetch + DOM logic
│ └── style.css # Optional styling
│
├── parser/
│ ├── input.cpp # C++ analyzer
│ └── json.hpp # JSON library for C++
│
├── input.txt # Generated input code
├── symbols.json # Token/symbol output
├── ast.json # AST output (future)
│
├── server.js # Node.js + Express backend
├── package.json # Project dependencies
├── .gitignore # Files to exclude
└── README.md # This documentation


## 🛠️ Setup Instructions

### 1️⃣ Install Dependencies
Make sure **Node.js** and a **C++ compiler** (e.g. MinGW on Windows) are installed.

Then, install required npm packages:
```bash
npm install express

### 2️⃣ Compile the C++ Analyzer
g++ -std=c++11 parser/input.cpp -o parser.exe

### 3️⃣ Start the Server
node server.js

### 4️⃣ Open the App
Go to http://localhost:3000 in your browser to launch the tool.


