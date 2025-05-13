const fs = require('fs');
const path = require('path');

// RLE encode a flat array of bytes
function rleEncode(data) {
    const rle = [];
    let i = 0;
    while (i < data.length) {
        let count = 1;
        while (i + count < data.length && data[i] === data[i + count] && count < 255) {
            count++;
        }
        rle.push(count, data[i]);
        i += count;
    }
    return rle;
}

// Extracts the array data from a C++ header file
function extractBitmapArrays(content) {
    // Matches: static const uint8_t PROGMEM name[size] = { ... };
    const regex = /static const uint8_t PROGMEM (\w+)\[(\d+)\] = \{([\s\S]*?)\};/g;
    const arrays = [];
    let match;
    while ((match = regex.exec(content)) !== null) {
        const name = match[1];
        const size = parseInt(match[2], 10);
        const body = match[3];
        // Extract all hex values
        const values = body.match(/0x[0-9a-fA-F]{2}/g) || [];
        const data = values.map(v => parseInt(v, 16));
        if (data.length === size) {
            arrays.push({ name, data });
        }
    }
    return arrays;
}

// Generates RLE C++ array code
function generateRLEArrayCode(name, rleData) {
    const sectionName = name + '_rle';
    const lines = [`static const uint8_t PROGMEM ${sectionName}[${rleData.length}] = {`];
    for (let i = 0; i < rleData.length; i += 16) {
        const chunk = rleData.slice(i, i + 16);
        const line = chunk.map(value => `0x${value.toString(16).padStart(2, '0')}`).join(', ');
        lines.push(`    ${line},`);
    }
    lines.push('};\n');
    return lines.join('\n');
}

// Main migration function
function migrateFolderToRLE(folderPath) {
    const parentDir = path.dirname(folderPath);
    const baseName = path.basename(folderPath);
    const rleFolder = path.join(parentDir, baseName + '_RLE');
    if (!fs.existsSync(rleFolder)) {
        fs.mkdirSync(rleFolder, { recursive: true });
    }
    const files = fs.readdirSync(folderPath);
    for (const file of files) {
        if (!file.endsWith('.h')) continue;
        const filePath = path.join(folderPath, file);
        const content = fs.readFileSync(filePath, 'utf8');
        const arrays = extractBitmapArrays(content);
        if (arrays.length === 0) continue;
        let newContent = content;
        // Remove any previously appended RLE arrays for this file
        newContent = newContent.replace(/static const uint8_t PROGMEM \w+_rle\[\d+\] = \{[\s\S]*?\};\s*/g, '');
        // Only output RLE arrays, not the original arrays
        let rleOnlyContent = '';
        for (const { name, data } of arrays) {
            const rleData = rleEncode(data);
            const rleCode = generateRLEArrayCode(name, rleData);
            rleOnlyContent += '\n' + rleCode;
        }
        // Write only the RLE arrays to the new file
        const outPath = path.join(rleFolder, file);
        fs.writeFileSync(outPath, rleOnlyContent.trimStart(), 'utf8');
        console.log(`Created: ${outPath}`);
    }
}

// Usage: node migrateBitmapToRLE.js <folder>
if (require.main === module) {
    const folder = process.argv[2] || './src/Bitmaps';
    if (!fs.existsSync(folder) || !fs.statSync(folder).isDirectory()) {
        console.error('Invalid folder:', folder);
        process.exit(1);
    }
    migrateFolderToRLE(folder);
    console.log('Migration complete.');
}
