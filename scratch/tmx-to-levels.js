/**
 * Reads all *.tmx files in this directory and writes levels.txt:
 *
 * #
 * <level name>
 * <width>,<height>
 * <csv tile data as in the TMX (trimmed outer whitespace)>
 *
 * Usage: node tmx-to-levels.js
 * Output: levels.txt (same directory)
 */

const fs = require('fs');
const path = require('path');

const SCRATCH_DIR = __dirname;
const OUT_FILE = path.join(SCRATCH_DIR, 'levels.txt');

/** Normalize CRLF / CR to LF (Node fs has no "read as LF" mode). */
function readUtf8Lf(filePath) {
  return fs.readFileSync(filePath, 'utf8').replace(/\r\n?/g, '\n');
}

const BRICK_TYPE_NAME_TO_LEVEL_ID = {
  BRICK_TYPE_NONE: 0,
  BRICK_TYPE_NORMAL: 1,
  BRICK_TYPE_METAL: 2,
  BRICK_TYPE_METAL_BOMB: 3,
  BRICK_TYPE_POWERUP_EXTRA_BALLS: 4,
  BRICK_TYPE_POWERUP_SHORT_PADDLE: 5,
  BRICK_TYPE_POWERUP_ARMOR: 6,
  BRICK_TYPE_POWERUP_TIMER: 7,
  BRICK_TYPE_POWERUP_METAL_BALLS: 8,
  BRICK_TYPE_BRITTLE: 9,
  BRICK_TYPE_SMALL_SQUARE_NORMAL: 10,
  BRICK_TYPE_SMALL_CIRCLE_NORMAL: 11,
  BRICK_TYPE_SMALL_SQUARE_METAL: 12,
  BRICK_TYPE_SMALL_CIRCLE_METAL: 13,
  BRICK_TYPE_VENT: 14,
  BRICK_TYPE_SMALL_VENT: 15,
  BRICK_TYPE_SMALL_INDESTRUCTIBLE: 16,
};
const BRICK_TYPE_TILESHEET_ID_TO_NAME = {
  '0,': 'BRICK_TYPE_NONE',
  '1,2,': 'BRICK_TYPE_NORMAL',
  '3,4,': 'BRICK_TYPE_METAL',
  '5,6,': 'BRICK_TYPE_METAL_BOMB',
  '7,8,': 'BRICK_TYPE_POWERUP_EXTRA_BALLS',
  '9,10,': 'BRICK_TYPE_POWERUP_SHORT_PADDLE',
  '11,12,': 'BRICK_TYPE_POWERUP_ARMOR',
  '13,14,': 'BRICK_TYPE_POWERUP_TIMER',
  '15,16,': 'BRICK_TYPE_POWERUP_METAL_BALLS',
  '17,18,': 'BRICK_TYPE_BRITTLE',
  '19,': 'BRICK_TYPE_SMALL_SQUARE_NORMAL',
  '20,': 'BRICK_TYPE_SMALL_CIRCLE_NORMAL',
  '21,': 'BRICK_TYPE_SMALL_SQUARE_METAL',
  '22,': 'BRICK_TYPE_SMALL_CIRCLE_METAL',
  '23,24,': 'BRICK_TYPE_VENT',
  '25,': 'BRICK_TYPE_SMALL_VENT',
  '26,': 'BRICK_TYPE_SMALL_INDESTRUCTIBLE',
};

function mapTagAttrs(xml) {
  const m = xml.match(/<map\b[^>]*>/);
  if (!m) throw new Error('Missing <map> element');
  const tag = m[0];
  const width = tag.match(/\bwidth="(\d+)"/);
  const height = tag.match(/\bheight="(\d+)"/);
  if (!width || !height) {
    throw new Error('Map missing width or height on <map> tag');
  }
  return { width: width[1], height: height[1] };
}

function mapDisplayName(xml, fileStem) {
  const props = xml.match(/<properties>\s*([\s\S]*?)\s*<\/properties>/i);
  if (!props) return fileStem;
  const nameProp = props[1].match(/<property\s+name="name"\s+value="([^"]*)"/i);
  return nameProp ? nameProp[1] : fileStem;
}

function firstCsvData(xml) {
  const dm = xml.match(/<data\s+encoding="csv"\s*>([\s\S]*?)<\/data>/i);
  if (!dm) throw new Error('No <data encoding="csv"> found');
  return dm[1].trim();
}

function csvDataLayers(xml) {
  const layers = [];
  const re = /<layer\b[\s\S]*?<data\s+encoding="csv"\s*>([\s\S]*?)<\/data>[\s\S]*?<\/layer>/gi;
  let m = null;
  while ((m = re.exec(xml)) !== null) {
    layers.push(m[1].trim());
  }
  return layers;
}

function parseCsvIds(csv) {
  return csv
    .replace(/\n/g, '')
    .trim()
    .split(',')
    .filter(id => id !== '');
}

function tilesheetIdToBrickName(tilesetId) {
  for (const [key, value] of Object.entries(BRICK_TYPE_TILESHEET_ID_TO_NAME)) {
    if (key.includes(tilesetId + ',')) {
      return value;
    }
  }
  return '';
}

function main() {
  const tmxFiles = fs
    .readdirSync(SCRATCH_DIR)
    .filter(f => f.toLowerCase().endsWith('.tmx'))
    .sort((a, b) =>
      a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base' })
    );

  if (tmxFiles.length === 0) {
    console.error('No .tmx files in', SCRATCH_DIR);
    process.exit(1);
  }

  const blocks = [];
  for (const file of tmxFiles) {
    const full = path.join(SCRATCH_DIR, file);
    const xml = readUtf8Lf(full);
    const stem = path.basename(file, path.extname(file));
    let attrs;
    let csv;
    try {
      attrs = mapTagAttrs(xml);
      const csvLayers = csvDataLayers(xml);
      if (csvLayers.length === 0) {
        throw new Error('No <layer> CSV data found');
      }
      const baseIds = parseCsvIds(csvLayers[0]);
      const dirIds = csvLayers[1] ? parseCsvIds(csvLayers[1]) : [];
      csv = baseIds
        .map((tilesetId, idx) => {
          const brickName = tilesheetIdToBrickName(tilesetId);
          if (!brickName) {
            throw new Error(`Unknown brick tile id: ${tilesetId}`);
          }
          const levelId = BRICK_TYPE_NAME_TO_LEVEL_ID[brickName];
          if (levelId === undefined) {
            throw new Error(`Unknown brick name: ${brickName}`);
          }
          if (
            brickName === 'BRICK_TYPE_VENT' ||
            brickName === 'BRICK_TYPE_SMALL_VENT'
          ) {
            const dirIdRaw = dirIds[idx];
            const dirId = dirIdRaw ? Number(dirIdRaw) : 30;
            const safeDirId = [30, 31, 32, 33, 34, 35].includes(dirId) ? dirId : 30;
            return `${levelId}:${safeDirId}`;
          }
          return levelId;
        })
        .join(',');
    } catch (e) {
      throw new Error(`${file}: ${e.message}`);
    }
    const name = mapDisplayName(xml, stem);
    blocks.push(['#', name, `${attrs.width},${attrs.height}`, csv].join('\n'));
  }

  const out = blocks.join('\n\n') + '\n';
  fs.writeFileSync(OUT_FILE, out, 'utf8');
  console.log(
    `Wrote ${OUT_FILE} (${tmxFiles.length} level(s): ${tmxFiles.join(', ')})`
  );
}

main();
