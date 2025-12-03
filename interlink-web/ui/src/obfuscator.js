const JavaScriptObfuscator = require('javascript-obfuscator');
const fs = require("fs");
const path = require("path");

let filepath = process.argv[2];
let files = [];

function ThroughDirectory(directory) {
    fs.readdirSync(directory).forEach(file => {
        if (file.substring(file.length-3) !== ".js") return;
        const Absolute = path.join(directory, file);
        if (fs.statSync(Absolute).isDirectory()) return ThroughDirectory(Absolute);
        else return files.push(Absolute);
    });
}

ThroughDirectory(filepath);

for (let f in files) {
    console.log(files[f])
    fs.readFile(files[f], function (err, data) {
        if (err) {
            return console.error(err);
        }
        let obfuscationResult = JavaScriptObfuscator.obfuscate(data.toString());
        fs.writeFile(files[f], obfuscationResult.getObfuscatedCode(),
            function (err) {
                if (err) {
                    return console.error(err);
                }
            });
    });
}
