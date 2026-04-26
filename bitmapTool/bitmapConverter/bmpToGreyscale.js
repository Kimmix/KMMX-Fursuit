import sharp from 'sharp';
import fs from 'node:fs';
import path from 'node:path';

// Get bitmapName from command line arguments, with fallback
const bitmapName = process.argv[2] || 'defaultBitmap';

// Directory containing GIF files
const inputDirectory = './input';

// Determine base output directory based on bitmap name
let baseDirectory = './output';
if (bitmapName.toLowerCase().includes('eye')) {
    baseDirectory = '../../src/Bitmaps/eyes';
} else if (bitmapName.toLowerCase().includes('mouth')) {
    baseDirectory = '../../src/Bitmaps/mouth';
}

// Output directory
const outputDirectory = `${baseDirectory}`;

// Ensure output directory exists
if (!fs.existsSync(outputDirectory)) {
    fs.mkdirSync(outputDirectory, { recursive: true });
}

// Output file path
const outputFile = `${outputDirectory}/${bitmapName}.h`;

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

    // Generate output file
    const content = generateOutputFileContent(allGreyscaleArrays);
    fs.writeFileSync(outputFile, content);
    console.log(`Output file generated: ${outputFile}`);
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
