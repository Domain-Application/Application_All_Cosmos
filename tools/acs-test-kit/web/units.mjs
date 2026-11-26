export default async function run(page, ui) {
  await page.waitForTimeout(4000);
  return await page.evaluate(() => [...document.querySelectorAll('tbody tr')].map(r => r.innerText.replace(/\s+/g,' ').trim()).filter(Boolean));
}
