const sharp = require('sharp');
const fs = require('fs');
const path = require('path');

// Directory containing GIF files
const inputDirectory = './input';

// Output file path
const outputFile = './output/output.txt';

// Process each GIF file in the directory
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

    const outputContent = generateOutputFileContent(allGreyscaleArrays);
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
        const sectionName = `mouthDefault${index}`;
        const sectionLines = [
            `static const uint8_t PROGMEM ${sectionName}[700] = {`
        ];

        for (let i = 0; i < data.length; i += 16) {
            const chunk = data.slice(i, i + 16);
            const line = chunk.map(value => `0x${value.toString(16).padStart(2, '0')}`).join(', ');
            sectionLines.push(`    ${line},`);
        }

        sectionLines.push(`};`);
        lines.push(...sectionLines, ''); // Empty line between sections
    }

    return lines.join('\n');
}
