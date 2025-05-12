const sharp = require('sharp');
const fs = require('fs');
const path = require('path');

// Get bitmapName and encoding type from command line arguments
const bitmapName = process.argv[2] || 'defaultBitmap';
const encoding = (process.argv[3] || 'raw').toLowerCase(); // 'raw' or 'rle'

const inputDirectory = './input';
const outputFile = `./output/${bitmapName}.h`;

fs.readdir(inputDirectory, async (err, files) => {
    if (err) {
        console.error('Error reading directory:', err);
        return;
    }

    const allGreyscaleArrays = [];
    for (const [index, file] of files.entries()) {
        if (path.extname(file).toLowerCase() === '.gif') {
            const inputPath = path.join(inputDirectory, file);
            const greyscaleArray = await processGif(inputPath);
            allGreyscaleArrays.push({ data: greyscaleArray, index });
        }
    }

    let outputContent;
    if (encoding === 'rle') {
        outputContent = generateRLEOutputFileContent(allGreyscaleArrays);
    } else {
        outputContent = generateOutputFileContent(allGreyscaleArrays);
    }
    fs.writeFileSync(outputFile, outputContent);
    console.log(`Merged output file generated: ${outputFile}`);
});

async function processGif(inputPath) {
    return new Promise((resolve, reject) => {
        sharp(inputPath)
            .grayscale()
            .raw()
            .toBuffer((err, buffer) => {
                if (err) {
                    console.error(`Error processing ${inputPath}:`, err);
                    reject(err);
                    return;
                }
                const greyscaleArray = Array.from(buffer);
                resolve(greyscaleArray);
            });
    });
}

function generateOutputFileContent(greyscaleArrays) {
    const lines = ['// Generated output'];
    for (const { data, index } of greyscaleArrays) {
        const sectionName = `${bitmapName}${index}`;
        const sectionLines = [
            `static const uint8_t PROGMEM ${sectionName}[576] = {`
        ];
        for (let i = 0; i < data.length; i += 16) {
            const chunk = data.slice(i, i + 16);
            const line = chunk.map(value => `0x${value.toString(16).padStart(2, '0')}`).join(', ');
            sectionLines.push(`    ${line},`);
        }
        sectionLines.push(`};`);
        lines.push(...sectionLines, '');
    }
    return lines.join('\n');
}

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

function generateRLEOutputFileContent(greyscaleArrays) {
    const lines = ['// Generated RLE output'];
    for (const { data, index } of greyscaleArrays) {
        const sectionName = `${bitmapName}${index}_rle`;
        const rleData = rleEncode(data);
        const sectionLines = [
            `static const uint8_t PROGMEM ${sectionName}[${rleData.length}] = {`
        ];
        for (let i = 0; i < rleData.length; i += 16) {
            const chunk = rleData.slice(i, i + 16);
            const line = chunk.map(value => `0x${value.toString(16).padStart(2, '0')}`).join(', ');
            sectionLines.push(`    ${line},`);
        }
        sectionLines.push(`};`);
        lines.push(...sectionLines, '');
    }
    return lines.join('\n');
}
