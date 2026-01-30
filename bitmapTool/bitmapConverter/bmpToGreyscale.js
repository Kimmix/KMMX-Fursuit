const sharp = require('sharp');
const fs = require('fs');
const path = require('path');

// Get bitmapName from command line arguments, with fallback
const bitmapName = process.argv[2] || 'defaultBitmap';
const encoding = (process.argv[3] || 'both').toLowerCase(); // 'raw', 'rle', or 'both'

// Directory containing GIF files
const inputDirectory = './input';

// Determine base output directory based on bitmap name
let baseDirectory = './output';
if (bitmapName.toLowerCase().includes('eye')) {
    baseDirectory = '../../src/Bitmaps/eyes';
} else if (bitmapName.toLowerCase().includes('mouth')) {
    baseDirectory = '../../src/Bitmaps/mouth';
}

// Separate directories for raw and RLE
const outputDirectoryRaw = `${baseDirectory}`;
const outputDirectoryRLE = `${baseDirectory}_RLE`;

// Ensure output directories exist
if (!fs.existsSync(outputDirectoryRaw)) {
    fs.mkdirSync(outputDirectoryRaw, { recursive: true });
}
if (!fs.existsSync(outputDirectoryRLE)) {
    fs.mkdirSync(outputDirectoryRLE, { recursive: true });
}

// Output file path(s)
const outputFileRaw = `${outputDirectoryRaw}/${bitmapName}.h`;
const outputFileRLE = `${outputDirectoryRLE}/${bitmapName}.h`;

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

    // Generate output based on encoding parameter
    if (encoding === 'both') {
        // Generate both raw and RLE versions
        const rawContent = generateOutputFileContent(allGreyscaleArrays);
        const rleContent = generateRLEOutputFileContent(allGreyscaleArrays);

        fs.writeFileSync(outputFileRaw, rawContent);
        console.log(`Raw output file generated: ${outputFileRaw}`);

        fs.writeFileSync(outputFileRLE, rleContent);
        console.log(`RLE output file generated: ${outputFileRLE}`);
    } else if (encoding === 'rle') {
        const rleContent = generateRLEOutputFileContent(allGreyscaleArrays);
        fs.writeFileSync(outputFileRLE, rleContent);
        console.log(`RLE output file generated: ${outputFileRLE}`);
    } else {
        const rawContent = generateOutputFileContent(allGreyscaleArrays);
        fs.writeFileSync(outputFileRaw, rawContent);
        console.log(`Raw output file generated: ${outputFileRaw}`);
    }
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
        const sectionName = `${bitmapName}${index}`;
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

function generateOutputFileContent(greyscaleArrays) {
    const lines = ['// Generated output'];

    for (const { data, index } of greyscaleArrays) {
        const sectionName = `${bitmapName}${index}`;
        const sectionLines = [
            `static const uint8_t PROGMEM ${sectionName}[${data.length}] = {`
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
