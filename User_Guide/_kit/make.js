// Build the AllCosmos link guide. Copy this file + build.js (cpp-user-guide-builder skill) + content/*.js into one
// work folder with screens/*.png in hq/, then:
//   node make.js            -> writes to the repo's User_Guide folder
//   node make.js <folder>   -> writes a test copy there (verify it, then run without an argument)
process.env.GUIDE_OUT_DIR = process.env.GUIDE_OUT_DIR || 'C:/Domain/_docs/Application_All_Cosmos/User_Guide';
const fs = require('fs'), path = require('path');
const B = require('./build');
const out = process.argv[2] || B.OUT;
const JOBS = [
  [() => require('./allcosmos_api')(), 'AllCosmos Link/ALLCOSMOS LINK USER GUIDE.docx'],
];
(async () => {
  for (const [mk, rel] of JOBS) {
    const f = path.join(out, rel);
    fs.mkdirSync(path.dirname(f), { recursive: true });
    try { await B.writeDocx(mk(), f); console.log('wrote', f); }
    catch (e) { console.log('FAILED', f, e.code || e.message, e.code === 'EBUSY' ? '(open in WPS/Word - ask the user to close it, or save as _v2)' : ''); }
  }
})();
