// Runs `code` (a JS function body) in the page's main world via a <script> tag; the body may `return` a value
// (sync or a Promise). The result is written to <pre id="__mw_result"> and read back from the isolated world.
export async function mainWorld(page, code, timeoutMs = 30000) {
  const tag = 'mw' + Date.now();
  await page.evaluate((t) => { const old = document.getElementById('__mw_result'); if (old) old.remove(); }, tag);
  await page.addScriptTag({ content: `(async () => { let out; try { out = await (async () => { ${code} })(); out = JSON.stringify(out ?? null); } catch (e) { out = JSON.stringify({ error: String(e && e.stack || e) }); } const p = document.createElement('pre'); p.id = '__mw_result'; p.style.display = 'none'; p.textContent = out; document.body.appendChild(p); })();` });
  await page.waitForSelector('#__mw_result', { state: 'attached', timeout: timeoutMs });
  const text = await page.evaluate(() => document.getElementById('__mw_result').textContent);
  return JSON.parse(text);
}
