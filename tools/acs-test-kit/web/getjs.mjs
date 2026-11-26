import fs from 'fs';
export default async function run(page, ui) {
  const src = await page.evaluate(async () => (await fetch([...document.scripts].find(s => /allcosmos\.js/.test(s.src)).src)).text());
  fs.writeFileSync('allcosmos.js', src);
  return src.length;
}
