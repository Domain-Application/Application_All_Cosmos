import fs from 'fs';
export default async function run(page, ui) {
  const srcs = await page.evaluate(() => [...document.scripts].map(s => s.src || ('INLINE:' + s.textContent.length)));
  const hits = await page.evaluate(() => [...document.scripts].filter(s => !s.src && /purchaseorder|despatchticket|Select First|selectFirst/i.test(s.textContent)).map(s => s.textContent));
  hits.forEach((h, i) => fs.writeFileSync(`inline${i}.js`, h));
  return { srcs, hits: hits.map(h => h.length) };
}
